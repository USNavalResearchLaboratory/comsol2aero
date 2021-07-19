// comsol2aero: a comsol mesh to frg aero mesh converter
//
// DISTRIBUTION STATEMENT A. Approved for public release; distribution is unlimited.
//
// AUTHORIZATION TO USE AND DISTRIBUTE
//
// By using or distributing the comsol2aero software ("THE SOFTWARE"), you agree to the following terms
// governing the use and redistribution of THE SOFTWARE originally developed at the U.S. Naval
// Research Laboratory (NRL), Computational Multiphysics SystemS Lab., Code 6394.
//
// The modules of comsol2aero containing an attribution in their header files to the NRL have been
// authored by federal employees. To the extent that a federal employee is an author of a portion of
// this software or a derivative work thereof, no copyright is claimed by the United States
// Government, as represented by the Secretary of the Navy ("GOVERNMENT") under Title 17, U.S. Code.
// All Other Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) source code distributions retain the above copyright notice, this list of conditions, and the
// following disclaimer in its entirety,
// (2) distributions including binary code include this paragraph in its entirety in the
// documentation or other materials provided with the distribution, and
// (3) all published research using this software display the following acknowledgment:
// "This work uses the software components contained within the NRL comsol2aero computer package written
// and developed by the U.S. Naval Research Laboratory, Computational Multiphysics Systems lab.,
// Code 6394"
//
// Neither the name of NRL or its contributors, nor any entity of the United States Government may
// be used to endorse or promote products derived from this software, nor does the inclusion of the
// NRL written and developed software directly or indirectly suggest NRL's or the United States
// Government's endorsement of this product.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR THE U.S. GOVERNMENT BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

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
