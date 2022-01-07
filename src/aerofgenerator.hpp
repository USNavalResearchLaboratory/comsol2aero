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

#ifndef AEROFGENERATOR_H
#define AEROFGENERATOR_H

#include "aeromesh.hpp"
#include "charstreamer.hpp"
#include "comsolmesh.hpp"
#include "config.hpp"

#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_real.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>

namespace aerof
{

using namespace boost::spirit::karma;

using namespace std;

using namespace aero;

using std::string;

// Formatting policy
template< typename Num >
struct RealPolicy : real_policies< Num >
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
using RealType = real_generator< double, RealPolicy< double > >;

// The core structure of the generator
template< typename OutputIterator >
struct GeneratorGrammar : grammar< OutputIterator, Mesh( ) >
{
  GeneratorGrammar( ) : GeneratorGrammar::base_type( mesh )
  {

    mesh = nodes << eol << elements << eol << omit[ attribute_labels ] << omit[ attributes ]
                 << omit[ matusage ] << topologies << eol;

    nodes %= "Nodes FluidNodes" << eol << eps[ _a = 1 ]
                                << ( lit( _a ) << eps[ ++_a ] << ' ' << ( real_ % ' ' ) ) % eol;

    elements %= "Elements FluidMesh_0 using FluidNodes"
                << eol << eps[ _a = 1 ] << ( lit( _a ) << eps[ ++_a ] << ' ' << element ) % eol;

    element = uint_ << ' ' << uint_ % ' ';

    attribute_labels
      %= ( "* Attributes/matusage labels"
           << eol << eps[ _a = 1 ]
           << ( "* " << lit( _a ) << eps[ ++_a ] << ' ' << boost::spirit::karma::string ) % eol )
         | eps;

    attributes %= "ATTRIBUTES" << eol << eps[ _a = 1 ]
                               << ( lit( _a ) << eps[ ++_a ] << ' ' << uint_ ) % eol;

    matusage %= "MATUSAGE" << eol << eps[ _a = 1 ]
                           << ( lit( _a ) << eps[ ++_a ] << ' ' << uint_ ) % eol;

    topologies
      %= ( topology_id << eol << eps[ _a = 1 ]
                       << ( lit( _a ) << eps[ ++_a ] << ' ' << ( uint_ << ' ' << uint_ % ' ' ) )
                            % eol )
         % eol;

    topology_id = "Elements " << boost::spirit::karma::string << "Surface_" << uint_
                              << " using FluidNodes";
  }

  rule< OutputIterator, Mesh( ) >                                      mesh;
  rule< OutputIterator, locals< size_t >, Mesh::Nodes( ) >             nodes;
  rule< OutputIterator, locals< size_t >, Mesh::Elements( ) >          elements;
  rule< OutputIterator, Mesh::Element( ) >                             element;
  rule< OutputIterator, locals< size_t >, Mesh::AttributeLabels( ) >   attribute_labels;
  rule< OutputIterator, locals< size_t >, Mesh::Attributes( ) >        attributes;
  rule< OutputIterator, locals< size_t >, Mesh::Attributes( ) >        matusage;
  rule< OutputIterator, locals< size_t >, Mesh::SurfaceTopologies( ) > topologies;
  rule< OutputIterator, Mesh::TopologyId( ) >                          topology_id;

  RealType const real_;
};

class Generator
{

public:
  Generator( bool verb, const Mesh& aero_mesh );

  void generate( string file_name ) const;

  template< class S >
  void generate( S& stream ) const
  {
    namespace karma = boost::spirit::karma;

    string output_string;

    typedef back_insert_iterator< string > Sink;

    Sink sink( output_string );

    GeneratorGrammar< Sink > g;

    if ( !karma::generate( sink, g, mesh ) )
    {
      throw runtime_error( "Aero mesh generation failed." );
    }

    stream << output_string;

    stdclog.print( "Aero mesh generation completed." );
  }

private:
  const Mesh& mesh;

  CharStreamer< ostream > stdclog;
#ifdef NDEBUG
  NoneCharStreamer< ostream > debugstdout;
#else
  char_streamer< ostream > debugstdout;
#endif
};

} // namespace aero
#endif // AEROFGENERATOR_H
