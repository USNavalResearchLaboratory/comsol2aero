// comsol2aero: a comsol mesh to frg aero mesh Converter

// AUTHORIZATION TO USE AND DISTRIBUTE. By using or distributing the comsol2aero software
// ("THE SOFTWARE"), you agree to the following terms governing the use and redistribution of
// THE SOFTWARE originally developed at the U.S. Naval Research Laboratory ("NRL"), Computational
// Multiphysics Systems Lab., Code 6394.

// The modules of comsol2aero containing an attribution in their header files to the NRL have been
// authored by federal employees. To the extent that a federal employee is an author of a portion of
// this software or a derivative work thereof, no copyright is claimed by the United States
// Government, as represented by the Secretary of the Navy ("GOVERNMENT") under Title 17, U.S. Code.
// All Other Rights Reserved.

// Download, redistribution and use of source and/or binary forms, with or without modification,
// constitute an acknowledgement and agreement to the following:

// (1) source code distributions retain the above notice, this list of conditions, and the
// following disclaimer in its entirety,
// (2) distributions including binary code include this paragraph in its entirety in the
// documentation or other materials provided with the distribution, and
// (3) all published research using this software display the following acknowledgment:
// "This work uses the software components contained within the NRL comsol2aero computer package
// written and developed by the U.S. Naval Research Laboratory, Computational Multiphysics Systems
// lab., Code 6394"

// Neither the name of NRL or its contributors, nor any entity of the United States Government may
// be used to endorse or promote products derived from this software, nor does the inclusion of the
// NRL written and developed software directly or indirectly suggest NRL's or the United States
// Government's endorsement of this product.

// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR THE U.S. GOVERNMENT BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// NOTICE OF THIRD-PARTY SOFTWARE LICENSES. This software uses open source software packages from
// third parties. These are available on an "as is" basis and subject to their individual license
// agreements. Additional information can be found in the provided "licenses" folder.

#ifndef CONVERTER_H
#define CONVERTER_H

#include "aeromesh.hpp"
#include "charstreamer.hpp"
#include "comsolmesh.hpp"
#include "config.hpp"

#include <array>
#include <memory>

namespace detail
{
typedef std::pair< std::size_t, std::size_t > supportedType;
}

// { Aero-s element type id, element order }. The element order is not currently used
static std::array< detail::supportedType, 2 > tri_supported = { { { 3, 1 }, { 4, 1 } } };

static std::array< detail::supportedType, 9 > tet_supported = { { { 5, 1 },
                                                                 { 23, 1 },
                                                                 //   {25,2},
                                                                 { 40, 1 },
                                                                 { 41, 1 },
                                                                 //   {42,2},
                                                                 { 50, 1 },
                                                                 { 311, 1 },
                                                                 { 331, 1 } } };

static std::array< detail::supportedType, 8 > pyr_supported = {
  { { 17, 1 }, { 44, 1 }, { 45, 1 }, { 51, 1 }, { 70, 1 }, { 82, 1 }, { 201, 1 }, { 202, 1 } }
};

static std::array< detail::supportedType, 3 > prism_supported
  = { { { 24, 1 }, { 83, 1 }, { 90, 1 } } };

static std::array< detail::supportedType, 8 > hex_supported = {
  { { 17, 1 }, { 44, 1 }, { 45, 1 }, { 51, 1 }, { 70, 1 }, { 82, 1 }, { 201, 1 }, { 202, 1 } }
};

/*! \brief Maps elements one mesh format to another.
 *
 *
 *  This is an abstract base class. get_to_id() member reflects the ID of the
 *  element in the converted mesh.
 */
template< class TOID >
class ElementMapper
{
public:
  ElementMapper( size_t to ) : to_node_count_( to )
  {
  }

  virtual const TOID& get_to_id( ) const = 0;

  aero::Mesh::Connectivity map( const comsol::ElementSet::Element& element ) const
  {
    aero::Mesh::Connectivity con;
    con.reserve( element.size( ) );

    for ( size_t k = 0; k != to_node_count_; k++ )
    {
      con.push_back( element[ map( k ) ] + 1 );
    }
    return con;
  }

protected:
  virtual const size_t& map( size_t i ) const = 0;

  size_t to_node_count_;
};

typedef ElementMapper< size_t > ComsolToAeroElementMapperBase;

/*! \brief Maps elements from comsol to aero.
 *
 *
 *  The get_to_id() member has a generic implementation that allows
 *  for setting the converted element id at object construction time.
 *
 *  The mapping is defined by a static array that most compilers should
 *  construct in compile time.
 */
template< size_t... mappings >
class ComsolToAeroElementMapper : public ComsolToAeroElementMapperBase
{
public:
  ComsolToAeroElementMapper( size_t aeroID ) :
    ComsolToAeroElementMapperBase( sizeof...( mappings ) ), aeroID_( aeroID )
  {
  }

  const size_t& get_to_id( ) const
  {
    return aeroID_;
  }

protected:
  const size_t& map( size_t i ) const
  {
    return node_mapping[ i ];
  }

private:
  std::size_t aeroID_;

  std::array< std::size_t, sizeof...( mappings ) > node_mapping { { mappings... } };
};

/*! \brief The mesh Converter
 *
 *
 *  The Converter will convert a comsol mesh to an aero mesh, given certain
 *  mapping options that define what will be the type id in aero of certain
 *  comsol element types.
 */
class Converter
{
public:
  Converter( bool                                        verb,
             bool                                        associate_selections_with_attributes,
             const std::map< std::string, std::size_t >& mapping_options,
             const std::vector< std::string >&           pr,
             const std::vector< std::string >&           accepted_selections );

  void convert( const comsol::Mesh& comsol_mesh, aero::Mesh& aero_mesh ) const;

private:
  using MapperPtr = std::shared_ptr< ComsolToAeroElementMapperBase >;
  using Mappers = std::map< std::string, MapperPtr >;

  bool selections_to_attributes = true;

  Mappers                  boundary_mappers; // Only surface ones in aero
  Mappers                  domain_mappers;
  std::vector< std::string > prefixes;
  std::vector< std::string > accepted_selections_;

  CharStreamer< std::ostream > std_clog;
#ifdef NDEBUG
  NoneCharStreamer< std::ostream > debug_stdout;
#else
  char_streamer< std::ostream > debugstdout;
#endif

  void map_3d_comsol_selections_to_aero_attributes(
    const comsol::Mesh::SelectionObjects&        selection_objects,
    aero::Mesh&                                  aero_mesh,
    std::size_t&                                 attribute_overwrites,
    const comsol::ElementSet::GeometricIndicies& geometry_set,
    std::size_t&                                 not_assigned ) const;

  void map_comsol_surface_selections_to_aero_surfacetopo(
    const comsol::Mesh::SelectionObjects& selection_objects,
    aero::Mesh&                           aero_mesh,
    const comsol::ElementSet&             geometry_set ) const;
};

#endif // CONVERTER_H
