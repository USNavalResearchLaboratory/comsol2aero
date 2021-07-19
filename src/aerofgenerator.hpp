// comsol2aero: a simple comsol mesh to frg aero mesh converter

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
  generator_grammar( ) : generator_grammar::base_type( mesh )
  {

    mesh = nodes << eol << elements << eol << omit[ attributes_labels ] << omit[ attributes ]
                 << omit[ matusage ] << topologies << eol;

    nodes %= "Nodes FluidNodes" << eol << eps[ _a = 1 ]
                                << ( lit( _a ) << eps[ ++_a ] << ' ' << ( real_ % ' ' ) ) % eol;

    elements %= "Elements FluidMesh_0 using FluidNodes"
                << eol << eps[ _a = 1 ] << ( lit( _a ) << eps[ ++_a ] << ' ' << element ) % eol;

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

    topologies
      %= ( topology_id << eol << eps[ _a = 1 ]
                       << ( lit( _a ) << eps[ ++_a ] << ' ' << ( uint_ << ' ' << uint_ % ' ' ) )
                            % eol )
         % eol;

    topology_id = "Elements " << boost::spirit::karma::string << "Surface_" << uint_
                              << " using FluidNodes";
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
  generator( bool verb, const mesh_t& aMesh );

  void generate( string fileName ) const;

  template< class S >
  void generate( S& stream ) const
  {
    namespace karma = boost::spirit::karma;

    string outputString;

    typedef back_insert_iterator< string > sink_t;

    sink_t sink( outputString );

    generator_grammar< sink_t > g;

    if ( !karma::generate( sink, g, mesh ) )
    {
      throw runtime_error( "Aero mesh generation failed." );
    }

    stream << outputString;

    stdclog.print( "Aero mesh generation completed." );
  }

private:
  const mesh_t& mesh;

  char_streamer< ostream > stdclog;
#ifdef NDEBUG
  none_char_streamer< ostream > debugstdout;
#else
  char_streamer< ostream > debugstdout;
#endif
};

} // namespace aero
#endif // AEROFGENERATOR_H
