#include "converter.hpp"
#include "comsolmesh.hpp"
#include "utils.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

// mappings
//                      Comsol id (minus one)----|--->
//                                    |
using TriMapper  = ComsolToAeroElementMapper< 2, 0, 1 >;
using QuadMapper = ComsolToAeroElementMapper< 2, 0, 1, 3 >;
using TetMapper  = ComsolToAeroElementMapper< 2, 0, 1, 3 >;
// typedef ComsolToAeroElementMapper< 2, 1, 0, 3 > tet_50_96_103_mapper; // special mapping for
// these types (probably not needed though)
using PyrMapper   = ComsolToAeroElementMapper< 4, 4, 4, 4, 2, 0, 1, 3 >;
using PrismMapper = ComsolToAeroElementMapper< 2, 0, 1, 5, 3, 4 >;
using HexMapper   = ComsolToAeroElementMapper< 6, 2, 3, 7, 4, 0, 1, 5 >;

template< class T >
shared_ptr< ComsolToAeroElementMapperBase > mapper( const string&                id,
                                                    const map< string, size_t >& mapping_options )
{
  using Ptr = shared_ptr< ComsolToAeroElementMapperBase >;

  auto iter = mapping_options.find( id );
  if ( iter != mapping_options.end( ) )
  {
    return Ptr( new T( iter->second ) );
  }
  else
  {
    stringstream ss;
    ss << "Unkown element type: " << id;
    throw runtime_error( ss.str( ) );
  }
}

bool is_surface_selection( const comsol::SelectionObject so )
{
  return so.dim_size == 2;
}

Converter::Converter( bool                         verb,
                      bool                         associate_selections_with_attributes,
                      const map< string, size_t >& mapping_options,
                      const std::vector< string >& pr,
                      const std::vector< string >& accepted_selections ) :
  selections_to_attributes( associate_selections_with_attributes ),
  prefixes( pr ), accepted_selections_( accepted_selections ), std_clog( clog, verb ),
  debug_stdout( cerr, true )
{

  using Ptr = shared_ptr< ComsolToAeroElementMapperBase >;

  // FIXME: These map tri, quad elements to surfacetopo. Will maybe need functionality to map to
  // domain elements.
  // FIXME: Test the output of selection for tri and quad
  boundary_mappers[ "tri" ]  = mapper< TriMapper >( "tri", mapping_options );
  boundary_mappers[ "quad" ] = Ptr( new QuadMapper( 1 ) );

  domain_mappers[ "tet" ] = mapper< TetMapper >( "tet", mapping_options );
  // domain_mappers[ "tet_50_96_103" ]   = mapper< tet_50_96_103_mapper >   ( "tet_50_96_103",
  // mapping_options );
  domain_mappers[ "pyr" ]   = mapper< PyrMapper >( "pyr", mapping_options );
  domain_mappers[ "prism" ] = mapper< PrismMapper >( "prism", mapping_options );
  domain_mappers[ "hex" ]   = mapper< HexMapper >( "hex", mapping_options );
}

void Converter::map_3d_comsol_selections_to_aero_attributes(
  const comsol::Mesh::SelectionObjects&        selection_objects,
  aero::Mesh&                                  aero_mesh,
  std::size_t&                                 attribute_overwrites,
  const comsol::ElementSet::GeometricIndicies& geometry_set,
  std::size_t&                                 not_assigned ) const
{
  auto                selection_id = geometry_set;
  std::vector< bool > already_set( selection_id.size( ), false );

  for ( std::size_t i = 0; i != selection_objects.size( ); i++ )
  {
    const auto& selection_object = selection_objects[ i ];

    if ( selection_object.dim_size == 2 ) // Do not assign selections of dimension 2. TODO: Maybe also disable dimension 1
    {
      continue;
    }

    std::size_t id = i;

    // TODO: Maybe make this if a bit more clear
    if ( accepted_selections_.size( ) != 0 )
    {
      auto iter = std::find(
        accepted_selections_.begin( ), accepted_selections_.end( ), selection_object.label );
      if ( iter != accepted_selections_.end( ) )
      {
        id = std::distance( accepted_selections_.begin( ), iter );
      }
      else
      {
        continue;
      }
    }

    std_clog.print( "Attribute conversion." );
    std_clog.print( "  Selection object: ", selection_object.label );
    std_clog.print( "    Number of entites: ", selection_object.entities.size( ) );

    for ( const auto entity : selection_object.entities )
    {
      for ( std::size_t j = 0; j != selection_id.size( ); j++ )
      {
        if ( geometry_set[ j ] == entity )
        {
          if ( already_set[ j ] )
          {
            attribute_overwrites++;
          }
          selection_id[ j ] = id + 1; // We are starting from 1 in mat definitions in the generator
          already_set[ j ]  = true;
        }
      }
    }
  }
  not_assigned += std::count( already_set.begin( ), already_set.end( ), false );

  copy( selection_id.begin( ), selection_id.end( ), back_inserter( aero_mesh.attributes ) );
}

void Converter::convert( const comsol::Mesh& comsol_mesh, aero::Mesh& aero_mesh ) const
{
  std_clog.print( "\nConverting mesh of comsol mesh to aero mesh...\n" );

  std_clog.print( "Converting nodes..." );

  const auto& coords = comsol_mesh.object.coordinates;

  std_clog.print( "  Number of nodes: ", coords.size( ) );

  copy( coords.begin( ), coords.end( ), back_inserter( aero_mesh.nodes ) );

  auto& surface_topologies = aero_mesh.surface_topologies;

  std::size_t attribute_overwrites = 0;
  std::size_t not_assigned         = 0;

  const auto& selection_objects = comsol_mesh.selection_object;

  std_clog.print( "Converting topology" );
  for ( size_t i = 0; i != comsol_mesh.object.element_sets.size( ); i++ )
  {

    const auto& elementSet    = comsol_mesh.object.element_sets[ i ];
    const auto& element_type  = elementSet.element_type;
    const auto& elementNameId = element_type.second;
    const auto& elements      = elementSet.elements;
    const auto& geometry_set  = elementSet.geometric_indicies;

    if ( elements.size( ) != geometry_set.size( ) )
    {
      throw runtime_error(
        "Geometric index size and element array size are not the same." ); // This is not supposed
                                                                           // to happen.
    }

    auto iter = domain_mappers.find( elementNameId );

    if ( iter != domain_mappers.end( ) )
    {
      MapperPtr mapper = iter->second;

      //            // This is kind of a hack. TODO: integrate this better
      //            if ( ( mapper->get_to_id() == 50 ) || ( mapper->get_to_id() == 96 ) || (
      //            mapper->get_to_id() == 103 ) )
      //            {
      //                mapper = domain_mappers.find( "tet_50_96_103" )->second;
      //            }

      std_clog.print( "Comsol type id: ",
                     elementNameId,
                     "(",
                     elements[ 0 ].size( ),
                     " nodes) to aero type id: ",
                     mapper->get_to_id( ) );
      std_clog.print( "  Number of elements: ", elements.size( ) );

      for ( size_t j = 0; j != elements.size( ); j++ )
      {
        auto connectivity = mapper->map( elements[ j ] );
        // Pushing elements
        aero_mesh.elements.push_back( aero::Mesh::Element( mapper->get_to_id( ), connectivity ) );
      }

      if ( !selections_to_attributes )
      {
        // Copying attributes of each element type ( the comsol domain ids )
        copy( geometry_set.begin( ), geometry_set.end( ), back_inserter( aero_mesh.attributes ) );
      }
      else
      {
        map_3d_comsol_selections_to_aero_attributes(
          selection_objects, aero_mesh, attribute_overwrites, geometry_set, not_assigned );
      }

      // map_comsol_surface_selections_to_aero_surfacetopo( selection_objects, aero_mesh, elementSet
      // );
    }
    else
    {
      iter = boundary_mappers.find( elementNameId );

      if ( iter != boundary_mappers.end( ) )
      {

        MapperPtr mapper = iter->second;

        std_clog.print( "Comsol type id: ",
                       elementNameId,
                       " to Aero surfacetopo type id: ",
                       mapper->get_to_id( ) );
        std_clog.print( "  Number of faces: ", geometry_set.size( ) );

        // Collect the  connectivity data on surface topologies
        for ( size_t j = 0; j != geometry_set.size( ); j++ )
        {
          auto connectivity = mapper->map( elements[ j ] );

          std::string prefix;
          if ( prefixes.size( ) != 0 )
          {
            if ( geometry_set[ j ] >= prefixes.size( ) )
            {
              throw std::invalid_argument(
                "Comsol geometry contains more surfaces than the number of surface names "
                "provided." );
            }
            prefix = prefixes[ geometry_set[ j ] ];
          }

          aero::Mesh::TopologyId id( prefix, geometry_set[ j ] + 1 );

          auto& geom = surface_topologies[ id ];
          geom.push_back( aero::Mesh::Element( mapper->get_to_id( ), connectivity ) );
        }

        // map_comsol_surface_selections_to_aero_surfacetopo( selection_objects, aero_mesh,
        // elementSet );
      }
      else
      {
        std_clog.print(
          "Warning: Element with Comsol id name: ", elementNameId, " is not currently supported." );
      }
    }

    if ( accepted_selections_.size( ) == 0 )
    {
      for ( const auto& selection : selection_objects )
      {
        aero_mesh.attribute_labels.push_back( selection.label );
      }
    }
    else
    {
      for ( const auto& label : accepted_selections_ )
        aero_mesh.attribute_labels.push_back( label );
    }
  }

  // Convert surface selections
  if ( selection_objects.size( ) != 0 )
  {
    std_clog.print( "Surface selections conversion." );
  }

  for ( std::size_t i = 0; i != selection_objects.size( ); i++ )
  {
    const auto& selection_object = selection_objects[ i ];

    if ( is_surface_selection( selection_object ) )
    {
      std_clog.print( "  Surface Selection: ", selection_object.label );
      std_clog.print( "    Entities: ", selection_object.entities.size( ) );

      aero_mesh.selection_surface_topologies.push_back( aero::Mesh::SelectionSurfaceTopology( ) );

      auto& selection_surface_topology = *( aero_mesh.selection_surface_topologies.rbegin( ) );

      selection_surface_topology.first = selection_object.label;
      auto& selection_surface_elements = selection_surface_topology.second;

      for ( const auto entityID : selection_object.entities )
      {
        for ( const auto& entity : surface_topologies )
        {
          if ( entity.first.second - 1 == entityID )
          {
            const auto& elems = entity.second;
            for ( const auto& e : elems )
            {
              selection_surface_elements.push_back( e );
            }
          }
        }
      }
    }
  }

  if ( attribute_overwrites )
  {
    std::cerr << "Warning: " << attribute_overwrites
              << " overwrites of element attributes. Later selection "
                 "sets were prioritized.\n";
  }
  if ( not_assigned )
  {
    std::cerr << "Warning: " << not_assigned
              << ", elements were not"
                 " assigned a selection.\n";
    if ( accepted_selections_.size( ) != 0 )
    {

      std::stringstream ss;

      ss << "Selection set does not cover all elements. Please"
            " make sure that the comsol selections include all "
            "possible elements and that command line arguments "
            "contain them. Alternatively do not provide any "
            "selection names in the -s command.";
      if ( selection_objects.size( ) == 0 )
      {
        ss << " No selections detected in comsol file."; // TODO: We could check this earlier.
      }
      else
      {
        ss << " Selections detected in comsol file follow";

        for ( const auto& selection_object : selection_objects )
        {
          ss << ", " << selection_object.label;
        }
      }
      throw std::invalid_argument( ss.str( ) );
    }
  }
}
