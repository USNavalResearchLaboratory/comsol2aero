// comsol2aero: a simple comsol mesh to frg aero mesh converter

#include "converter.hpp"
#include "comsolmesh.hpp"
#include "utils.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

// mappings
//           Comsol id (minus one)----|--->
//                                    |
typedef comsol_to_aero_element_mapper< 2, 0, 1 >    tri_mapper;
typedef comsol_to_aero_element_mapper< 2, 0, 1, 3 > quad_mapper;
typedef comsol_to_aero_element_mapper< 2, 0, 1, 3 > tet_mapper;
// typedef comsol_to_aero_element_mapper< 2, 1, 0, 3 > tet_50_96_103_mapper; // special mapping for
// these types (probably not needed though)
typedef comsol_to_aero_element_mapper< 4, 4, 4, 4, 2, 0, 1, 3 > pyr_mapper;
typedef comsol_to_aero_element_mapper< 2, 0, 1, 5, 3, 4 >       prism_mapper;
typedef comsol_to_aero_element_mapper< 6, 2, 3, 7, 4, 0, 1, 5 > hex_mapper;

template< class T >
shared_ptr< comsol_to_aero_element_mapper_base > mapper(
  const string& id, const map< string, size_t >& mappingOptions )
{
  typedef shared_ptr< comsol_to_aero_element_mapper_base > ptr_t;

  auto iter = mappingOptions.find( id );
  if ( iter != mappingOptions.end( ) )
  {
    return ptr_t( new T( iter->second ) );
  }
  else
  {
    stringstream ss;
    ss << "Unkown element type: " << id;
    throw runtime_error( ss.str( ) );
  }
}

converter::converter( bool                         verb,
                      bool                         associate_selections_with_attributes,
                      const map< string, size_t >& mappingOptions,
                      const std::vector< string >& pr,
                      const std::vector< string >& accepted_selections ) :
  selections_to_attributes( associate_selections_with_attributes ),
  stdclog( clog, verb ), debugstdout( cerr, true ), prefixes( pr ),
  accepted_selections_( accepted_selections )
{

  typedef shared_ptr< comsol_to_aero_element_mapper_base > ptr_t;

  // FIXME: These map tri, quad elements to surfacetopo. Will maybe need functionality to map to
  // domain elements.

  boundaryMappers[ "tri" ]  = mapper< tri_mapper >( "tri", mappingOptions );
  boundaryMappers[ "quad" ] = ptr_t( new quad_mapper( 1 ) );

  domainMappers[ "tet" ] = mapper< tet_mapper >( "tet", mappingOptions );
  // domainMappers[ "tet_50_96_103" ]   = mapper< tet_50_96_103_mapper >   ( "tet_50_96_103",
  // mappingOptions );
  domainMappers[ "pyr" ]   = mapper< pyr_mapper >( "pyr", mappingOptions );
  domainMappers[ "prism" ] = mapper< prism_mapper >( "prism", mappingOptions );
  domainMappers[ "hex" ]   = mapper< hex_mapper >( "hex", mappingOptions );
}

void converter::map_comsol_selections_to_aero_attributes(
  const comsol::mesh_t::selection_objects_t&     selectionObjects,
  aero::mesh_t&                                  aMesh,
  std::size_t&                                   attribute_overwrites,
  const comsol::element_set::geometric_indicies& geometrySet,
  std::size_t&                                   not_assigned ) const
{
  auto                selectionId = geometrySet;
  std::vector< bool > alreadySet( selectionId.size( ), false );

  for ( auto i = 0; i != selectionObjects.size( ); i++ )
  {
    const auto& selectionObject = selectionObjects[ i ];

    std::size_t id = i;

    // TODO: Maybe make this if a bit more clear
    if ( accepted_selections_.size( ) != 0 )
    {
      auto iter = std::find(
        accepted_selections_.begin( ), accepted_selections_.end( ), selectionObject.label );
      if ( iter != accepted_selections_.end( ) )
      {
        id = std::distance( accepted_selections_.begin( ), iter );
      }
      else
      {
        continue;
      }
    }

    stdclog.print( "Selection object: ", selectionObject.label );

    for ( const auto entity : selectionObject.entities )
    {
      for ( auto j = 0; j != selectionId.size( ); j++ )
      {
        if ( geometrySet[ j ] == entity )
        {
          if ( alreadySet[ j ] )
          {
            attribute_overwrites++;
          }
          selectionId[ j ] = id + 1; // We are starting from 1 in mat definitions in the generator
          alreadySet[ j ]  = true;
        }
      }
    }
  }
  not_assigned += std::count( alreadySet.begin( ), alreadySet.end( ), false );

  copy( selectionId.begin( ), selectionId.end( ), back_inserter( aMesh.attributes ) );
}

void converter::convert( const comsol::mesh_t& cMesh, aero::mesh_t& aMesh ) const
{
  stdclog.print( "\nConverting mesh of comsol mesh to aero mesh...\n" );

  stdclog.print( "Converting nodes..." );

  const auto& coords = cMesh.object.coordinates;

  stdclog.print( "  Number of nodes: ", coords.size( ) );

  copy( coords.begin( ), coords.end( ), back_inserter( aMesh.nodes ) );

  auto& surfaceTopologies = aMesh.surfaceTopologies;

  std::size_t attribute_overwrites = 0;
  std::size_t not_assigned         = 0;

  const auto& selectionObjects = cMesh.selection_objects;

  stdclog.print( "Converting topology" );
  for ( size_t i = 0; i != cMesh.object.elementSets.size( ); i++ )
  {

    const auto& elementSet    = cMesh.object.elementSets[ i ];
    const auto& elementType   = elementSet.elementType;
    const auto& elementNameId = elementType.second;
    const auto& elements      = elementSet.elements;
    const auto& geometrySet   = elementSet.geometricIndicies;

    if ( elements.size( ) != geometrySet.size( ) )
    {
      throw runtime_error(
        "Geometric index size and element array size are not the same." ); // This is not supposed
                                                                           // to happen.
    }

    auto iter = domainMappers.find( elementNameId );

    if ( iter != domainMappers.end( ) )
    {
      mapper_ptr mapper = iter->second;

      //            // This is kind of a hack. TODO: integrate this better
      //            if ( ( mapper->getToID() == 50 ) || ( mapper->getToID() == 96 ) || (
      //            mapper->getToID() == 103 ) )
      //            {
      //                mapper = domainMappers.find( "tet_50_96_103" )->second;
      //            }

      stdclog.print( "Comsol type id: ",
                     elementNameId,
                     "(",
                     elements[ 0 ].size( ),
                     " nodes) to aero type id: ",
                     mapper->getToID( ) );
      stdclog.print( "  Number of elements: ", elements.size( ) );

      for ( size_t j = 0; j != elements.size( ); j++ )
      {
        auto connectivity = mapper->map( elements[ j ] );
        // Pushing elements
        aMesh.elements.push_back( aero::mesh_t::element_t( mapper->getToID( ), connectivity ) );
      }

      if ( !selections_to_attributes )
      {
        // Copying attributes of each element type ( the comsol domain ids )
        copy( geometrySet.begin( ), geometrySet.end( ), back_inserter( aMesh.attributes ) );
      }
      else
      {
        map_comsol_selections_to_aero_attributes(
          selectionObjects, aMesh, attribute_overwrites, geometrySet, not_assigned );
      }
    }
    else
    {
      iter = boundaryMappers.find( elementNameId );

      if ( iter != boundaryMappers.end( ) )
      {

        mapper_ptr mapper = iter->second;

        stdclog.print(
          "Comsol type id: ", elementNameId, " to Aero surfacetopo type id: ", mapper->getToID( ) );
        stdclog.print( "  Number of faces: ", geometrySet.size( ) );

        // Collect the  connectivity data on surface topologies
        for ( size_t j = 0; j != geometrySet.size( ); j++ )
        {
          auto connectivity = mapper->map( elements[ j ] );

          std::string prefix;
          if ( prefixes.size( ) != 0 )
          {
            if ( geometrySet[ j ] >= prefixes.size( ) )
            {
              throw std::invalid_argument(
                "Comsol geometry contains more surfaces than the number of surface names "
                "provided." );
            }
            prefix = prefixes[ geometrySet[ j ] ];
          }

          aero::mesh_t::topology_id_t id( prefix, geometrySet[ j ] + 1 );

          auto& geom = surfaceTopologies[ id ];
          geom.push_back( aero::mesh_t::element_t( mapper->getToID( ), connectivity ) );
        }
      }
      else
      {
        stdclog.print(
          "Warning: Element with Comsol id name: ", elementNameId, " is not currently supported." );
      }
    }

    if ( accepted_selections_.size( ) == 0 )
    {
      for ( const auto& selection : selectionObjects )
      {
        aMesh.attribute_labels.push_back( selection.label );
      }
    }
    else
    {
      for ( const auto& label : accepted_selections_ )
        aMesh.attribute_labels.push_back( label );
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
      if ( selectionObjects.size( ) == 0 )
      {
        ss << " No selections detected in comsol file."; // TODO: We could check this earlier.
      }
      else
      {
        ss << " Selections detected in comsol file follow";

        for ( const auto& selectionObject : selectionObjects )
        {
          ss << ", " << selectionObject.label;
        }
      }
      throw std::invalid_argument( ss.str( ) );
    }
  }
}
