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

#ifndef AEROSGENERATOR_H
#define AEROSGENERATOR_H

#include "aeromesh.hpp"
#include "charstreamer.hpp"
#include "comsolmesh.hpp"
#include "config.hpp"

#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_real.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>

namespace aeros
{

using namespace boost::spirit::karma;

using namespace std;

using namespace aero;

using std::string;

// Formatting policy
template< typename Num >
struct real_policy : real_policies< Num >
{
  static int floatfield( Num )
  {
    return real_policies< Num >::fmtflags::fixed;
  }

  static bool trailing_zeros( Num )
  {
    return true;
  }

  static unsigned precision( Num )
  {
    return 24;
  }
};

// Formatted double generator
typedef real_generator< double, real_policy< double > > real_type;

// The core structure of the generator
template< typename OutputIterator >
struct generator_grammar : grammar< OutputIterator, mesh_t( ) >
{
  generator_grammar( bool generate_matusage ) : generator_grammar::base_type( mesh )
  {

    mesh = "* Created with comsol2aero version "
           << lit( VERSION ) << eol << '*' << eol << nodes << eol << '*' << eol << elements << eol
           << '*' << eol << attributes_labels << eol << '*' << eol << attributes << eol << '*'
           << eol << ( ( eps( generate_matusage == true ) << matusage << eol << '*' << eol ) | eps )
           << topologies << eol;

    nodes %= "NODES" << eol << eps[ _a = 1 ]
                     << ( lit( _a ) << eps[ ++_a ] << ' ' << ( real_ % ' ' ) ) % eol;

    elements %= "TOPOLOGY" << eol << eps[ _a = 1 ]
                           << ( lit( _a ) << eps[ ++_a ] << ' ' << element ) % eol;

    element = uint_ << ' ' << uint_ % ' ';

    attributes_labels
      %= ( "* Attributes/matusage labels"
           << eol << eps[ _a = 1 ]
           << ( "* " << lit( _a ) << eps[ ++_a ] << ' ' << boost::spirit::karma::string ) % eol )
         | eps;

    attributes %= "ATTRIBUTES" << eol << eps[ _a = 1 ]
                               << ( lit( _a ) << eps[ ++_a ] << ' ' << uint_ ) % eol;

    matusage %= "MATUSAGE" << eol << eps[ _a = 1 ]
                           << ( lit( _a ) << eps[ ++_a ] << ' ' << uint_ ) % eol;

    topologies %= ( "SURFACETOPO "
                    << topology_id << eol << eps[ _a = 1 ]
                    << ( lit( _a ) << eps[ ++_a ] << ' ' << ( uint_ << ' ' << uint_ % ' ' ) ) % eol
                    << eol << '*' )
                    % eol
                  | eps;

    topology_id = omit[ boost::spirit::karma::string ] << uint_;
  }

  rule< OutputIterator, mesh_t( ) >                                       mesh;
  rule< OutputIterator, locals< size_t >, mesh_t::nodes_t( ) >            nodes;
  rule< OutputIterator, locals< size_t >, mesh_t::elements_t( ) >         elements;
  rule< OutputIterator, mesh_t::element_t( ) >                            element;
  rule< OutputIterator, locals< size_t >, mesh_t::attribute_labels_t( ) > attributes_labels;
  rule< OutputIterator, locals< size_t >, mesh_t::attributes_t( ) >       attributes;
  rule< OutputIterator, locals< size_t >, mesh_t::attributes_t( ) >       matusage;
  rule< OutputIterator, locals< size_t >, mesh_t::surface_topologies( ) > topologies;
  rule< OutputIterator, mesh_t::topology_id_t( ) >                        topology_id;

  real_type const real_;
};

class generator
{

public:
  generator( bool verb, bool matusage, const mesh_t& aMesh );

  void generate( string fileName ) const;

  template< class S >
  void generate( S& stream ) const
  {
    namespace karma = boost::spirit::karma;

    string outputString;

    typedef back_insert_iterator< string > sink_t;

    sink_t sink( outputString );

    generator_grammar< sink_t > g( matusage_ );

    if ( !karma::generate( sink, g, mesh ) )
    {
      throw runtime_error( "Aero mesh generation failed." );
    }

    stream << outputString;

    stdclog.print( "Aero mesh generation completed." );
  }

private:
  const mesh_t& mesh;
  bool          matusage_;

  char_streamer< ostream > stdclog;
#ifdef NDEBUG
  none_char_streamer< ostream > debugstdout;
#else
  char_streamer< ostream > debugstdout;
#endif
};

} // namespace aero
#endif // AEROSGENERATOR_H
