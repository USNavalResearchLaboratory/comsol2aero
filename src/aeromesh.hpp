// comsol2aero: a simple comsol mesh to frg aero mesh converter

#ifndef AEROMESH_HPP
#define AEROMESH_HPP

#include <boost/fusion/include/adapt_struct.hpp>
#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/fusion/adapted.hpp>

#include <map>
#include <string>
#include <utility>
#include <vector>
namespace aero
{

struct mesh_t
{
  using node_t             = std::vector< double >;
  using nodes_t            = std::vector< node_t >;
  using connectivity_t     = std::vector< std::size_t >;
  using element_t          = std::pair< std::size_t, connectivity_t >;
  using elements_t         = std::vector< element_t >;
  using attribute_labels_t = std::vector< std::string >;
  using attributes_t       = std::vector< std::size_t >;
  using topology_id_t      = std::pair< std::string, std::size_t >;
  using surface_topologies = std::map< topology_id_t, elements_t >;

  nodes_t            nodes;
  elements_t         elements;
  attribute_labels_t attribute_labels;
  attributes_t       attributes;
  surface_topologies surfaceTopologies;
};

} // namespace aero

BOOST_FUSION_ADAPT_STRUCT( aero::mesh_t,
                           ( aero::mesh_t::nodes_t, nodes )( aero::mesh_t::elements_t, elements )(
                             aero::mesh_t::attribute_labels_t,
                             attribute_labels )( aero::mesh_t::attributes_t, attributes )(
                             aero::mesh_t::attributes_t,
                             attributes ) // Repeating in case the user wants to output matusage
                           ( aero::mesh_t::surface_topologies, surfaceTopologies ) )

#endif // AEROMESH_HPP
