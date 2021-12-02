// comsol2aero: a comsol mesh to frg aero mesh converter

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

// NOTICE OF THIRD-PARTY SOFTWARE LICENSES. This software uses open source software packages from third
// parties. These are available on an "as is" basis and subject to their individual license agreements.
// Additional information can be found in the provided "licenses" folder.

#ifndef COMSOLMODEL_HPP
#define COMSOLMODEL_HPP

#include <boost/fusion/include/adapt_struct.hpp>

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace comsol
{

using namespace std;

// _t is used to denotate that this is a C++ type. _type conveys its actual
// semantics. For example element_type is the actual element type (e.g. tri,
// tet etc., while element_t is the C++ type that represents an element.

struct element_set
{
  using element_type       = pair< size_t, string >;
  using element_t          = vector< size_t >;
  using elements_t         = vector< element_t >;
  using geometric_indicies = vector< size_t >;

  element_type       elementType;
  elements_t         elements;
  geometric_indicies geometricIndicies;
};

} // namespace comsol

BOOST_FUSION_ADAPT_STRUCT( comsol::element_set,
                           ( comsol::element_set::element_type,
                             elementType )( comsol::element_set::elements_t,
                                            elements )( comsol::element_set::geometric_indicies,
                                                        geometricIndicies ) )

namespace comsol
{

struct mesh_object
{
  using element_sets = vector< element_set >;
  using point_t      = vector< double >; // FIXME: We could possibly restrict
                                         // this to 2 or 3 dim arrays, but
                                         // performance gains may be minimal.
  using coords_t = vector< point_t >;

  size_t classId;

  size_t version;
  size_t spaceDimensions;
  size_t numMeshPoints;
  size_t index0;

  coords_t coordinates;

  element_sets elementSets; // FIXME: Are elementSets part of mesh objects or
                            // they are separate entities?
};

} // namespace comsol

BOOST_FUSION_ADAPT_STRUCT(
  comsol::mesh_object,
  ( size_t, classId )( size_t, version )( size_t, spaceDimensions )( size_t, numMeshPoints )(
    size_t, index0 )( comsol::mesh_object::coords_t,
                      coordinates )( comsol::mesh_object::element_sets, elementSets ) )

namespace comsol
{

struct selection_object_t
{
  using entities_t = vector< size_t >;

  size_t     classId;
  size_t     version;
  string     label;
  size_t     dimsize;
  entities_t entities;
};

} // namespace comsol

BOOST_FUSION_ADAPT_STRUCT( comsol::selection_object_t,
                           ( size_t, classId )( size_t, version )( std::string, label )( size_t, dimsize )(
                             comsol::selection_object_t::entities_t, entities ) )

namespace comsol
{

struct mesh_t
{
  using version_t = pair< size_t, size_t >;

  using tag_t  = pair< size_t, string >;
  using tags_t = vector< tag_t >;

  using type_t  = pair< size_t, string >;
  using types_t = vector< type_t >;

  using selection_objects_t = vector< selection_object_t >;

  string              created;
  version_t           version;
  tags_t              tags;
  types_t             types;
  mesh_object         object;
  selection_objects_t selection_objects;
};

} // namespace comsol

BOOST_FUSION_ADAPT_STRUCT( comsol::mesh_t,
                           ( std::string, created )( comsol::mesh_t::version_t, version )(
                             comsol::mesh_t::tags_t, tags )( comsol::mesh_t::types_t, types )(
                             comsol::mesh_object, object )( comsol::mesh_t::selection_objects_t,
                                                            selection_objects ) )

#endif // COMSOLMODEL_HPP
