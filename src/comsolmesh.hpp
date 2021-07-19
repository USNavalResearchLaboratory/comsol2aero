// comsol2aero: a simple comsol mesh to frg aero mesh converter

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
  entities_t entities;
};

} // namespace comsol

BOOST_FUSION_ADAPT_STRUCT( comsol::selection_object_t,
                           ( size_t, classId )( size_t, version )( std::string, label )(
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
