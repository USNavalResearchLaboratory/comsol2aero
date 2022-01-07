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

// postfix Type conveys the semantic type and not the C++ type.
// Element is the C++ type that represents a finite element.

struct ElementSet
{
  using ElementType       = pair< size_t, string >;
  using Element           = vector< size_t >;
  using Elements          = vector< Element >;
  using GeometricIndicies = vector< size_t >;

  ElementType       element_type;
  Elements          elements;
  GeometricIndicies geometric_indicies;
};

} // namespace comsol

// clang-format off
BOOST_FUSION_ADAPT_STRUCT( comsol::ElementSet,
  ( comsol::ElementSet::ElementType,       element_type )
  ( comsol::ElementSet::Elements,          elements )
  ( comsol::ElementSet::GeometricIndicies, geometric_indicies ) )
// clang-format on

namespace comsol
{

struct MeshObject
{
  using ElementSets = vector< ElementSet >;
  using Point       = vector< double >; // FIXME: We could possibly restrict
                                        // this to 2 or 3 dim arrays, but
                                        // performance gains may be minimal.
  using Coords = vector< Point >;

  size_t class_id;

  size_t version;
  size_t space_dimensions;
  size_t num_mesh_points;
  size_t index0;

  Coords coordinates;

  ElementSets element_sets; // FIXME: Are element_sets part of mesh objects or
                            // they are separate entities?
};

} // namespace comsol

// clang-format off
BOOST_FUSION_ADAPT_STRUCT( comsol::MeshObject,
  ( size_t,                          class_id )
  ( size_t,                          version )
  ( size_t,                          space_dimensions )
  ( size_t,                          num_mesh_points )
  ( size_t,                          index0 )
  ( comsol::MeshObject::Coords,      coordinates )
  ( comsol::MeshObject::ElementSets, element_sets )
)
// clang-format on

namespace comsol
{

struct SelectionObject
{
  using Entities = vector< size_t >;

  size_t   class_id;
  size_t   version;
  string   label;
  size_t   dim_size;
  Entities entities;
};

} // namespace comsol

// clang-format off
BOOST_FUSION_ADAPT_STRUCT( comsol::SelectionObject,
  ( size_t,                            class_id )
  ( size_t,                            version )
  ( std::string,                       label )
  ( size_t,                            dim_size )
  ( comsol::SelectionObject::Entities, entities )
)
// clang-format on

namespace comsol
{

struct Mesh
{
  using Version = pair< size_t, size_t >;

  using Tag  = pair< size_t, string >;
  using Tags = vector< Tag >;

  using Type  = pair< size_t, string >;
  using Types = vector< Type >;

  using SelectionObjects = vector< SelectionObject >;

  string           created;
  Version          version;
  Tags             tags;
  Types            types;
  MeshObject       object;
  SelectionObjects selection_object;
};

} // namespace comsol

// clang-format off
BOOST_FUSION_ADAPT_STRUCT( comsol::Mesh,
  ( std::string,                    created )
  ( comsol::Mesh::Version,          version )
  ( comsol::Mesh::Tags,             tags )
  ( comsol::Mesh::Types,            types )
  ( comsol::MeshObject,             object )
  ( comsol::Mesh::SelectionObjects, selection_object )
)
// clang-format on
#endif // COMSOLMODEL_HPP
