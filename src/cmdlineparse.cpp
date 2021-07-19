// comsol2aero: a comsol mesh to frg aero mesh converter

#include "cmdlineparse.hpp"
#include "charstreamer.hpp"
#include "config.hpp"
#include "converter.hpp"
#include "detecttty.hpp"

#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <exception>
#include <iostream>
#include <stdexcept>

namespace std
{
std::ostream& operator<<( std::ostream& os, const vector< string >& vec )
{
  for ( const auto &item : vec )
  {
    os << item << " ";
  }
  return os;
}
}

using namespace std;

namespace po = boost::program_options;

// Generic element mapping option validation function
template< typename T >
void validate( boost::any& v, const std::vector< std::string >& values, T*, int )
{
  using namespace boost::program_options;

  validators::check_first_occurrence( v );

  const string& s = validators::get_single_string( values );

  std::size_t val = boost::lexical_cast< std::size_t >( s );

  auto f = std::find_if( T::supported( ).begin( ), T::supported( ).end( ), [ &val ]( auto t ) {
    return t.first == val;
  } );

  if ( f == T::supported( ).end( ) )
  {
    throw validation_error( validation_error::invalid_option_value );
  }

  v = boost::any( T( val ) );
}

template< class Derived >
struct mapping_option
{

  mapping_option( ) : value( 0 )
  {
  }

  mapping_option( std::size_t const& val ) : value( val )
  {
  }

  std::string helpText( )
  {
    const auto&       supported = Derived::supported( );
    std::stringstream ss;
    ss << Derived::name( ) << " element mapping. Valid values: ";
    ss << supported[ 0 ].first;
    for ( std::size_t i = 1; i != supported.size( ); i++ )
    {
      ss << ", " << supported[ i ].first;
    }

    return ss.str( );
  }

  std::size_t value;
};

template< class T >
std::ostream& operator<<( std::ostream& stream, const mapping_option< T >& h )
{
  stream << h.value;
  return stream;
}

//---

struct tri_t : public mapping_option< tri_t >
{
  using mapping_option::mapping_option;

  static std::string name( )
  {
    return "Triangular";
  }

  static auto supported( ) -> decltype( triSupported )
  {
    return triSupported;
  }
};

struct tet_t : public mapping_option< tet_t >
{
  using mapping_option::mapping_option;

  static std::string name( )
  {
    return "Tetrahedral";
  }

  static auto supported( ) -> decltype( tetSupported )
  {
    return tetSupported;
  }
};

template void validate< tet_t >( boost::any& v, const std::vector< std::string >&, tet_t*, int );

//---
struct pyr_t : public mapping_option< pyr_t >
{
  using mapping_option::mapping_option;

  static std::string name( )
  {
    return "Pyramidal pentahedral (degenerate)";
  }

  static auto supported( ) -> decltype( pyrSupported )
  {
    return pyrSupported;
  }
};

template void validate< pyr_t >( boost::any& v, const std::vector< std::string >&, pyr_t*, int );

//---
struct prism_t : public mapping_option< prism_t >
{
  using mapping_option::mapping_option;

  static std::string name( )
  {
    return "Prismatic pentahedral";
  }

  static auto supported( ) -> decltype( prismSupported )
  {
    return prismSupported;
  }
};

template void validate< prism_t >( boost::any& v,
                                   const std::vector< std::string >&,
                                   prism_t*,
                                   int );

//---
struct hex_t : public mapping_option< hex_t >
{
  using mapping_option::mapping_option;

  static std::string name( )
  {
    return "Hexahedral";
  }

  static auto supported( ) -> decltype( hexSupported )
  {
    return hexSupported;
  }
};

template void validate< hex_t >( boost::any& v, const std::vector< std::string >&, hex_t*, int );

user_options parseCommandLineOptions( int ac, char* av[] )
{

  user_options options;

  po::options_description desc(
    "Usage: comsol2aero [OPTION]... FILE\n"
    "Convert a comsol mesh to an aero mesh. Can operate on files, standard input and standard "
    "output.\n"
    "\nTo generate a mesh in comsol:\n"
    "1. Right click on the Mesh of interest (i.e. Mesh 1)\n"
    "2. Choose \"Export to file\".\n"
    "3. In \"File type\" choose \"COMSOL Multiphysics text (.mphtxt)\"\n"
    "4. Type in a filename.\n"
    "5. Make sure the Geometric entities options is enabled.\n"
    "6. Make sure the selections export is selected if you plan to use the selections feature of "
    "the converter.\n"
    "7. Click \"Export\"\n"
    "\nSupported elements:\n"
    "Comsol: 8 node hexahedral, 5 node pyramidal, 4 node tetrahedral, 4 node quadrilateral, 3 node "
    "triangular\n"
    "Aero:   See list in \"Element mapping\" below.\n"
    "\nExamples:\n"
    "comsol2aero comsolmesh.mphtxt\n"
    "comsol2aero -vf comsolmesh.mphtxt\n"
    "comsol2aero -v -o aero.mesh comsolmesh.mphtxt\n"
    "cat comsol_mesh.mphtxt | comsol2aero\n"
    "comsol2aero barmesh_dense.mphtxt -o barmesh.top --tet 5 --tri 4 -e -n InletFixed StickFixed "
    "StickFixed StickFixed StickFixed OutletFixed\n"
    "comsol2aero selections.mphtxt -o selections.geom -s \"Center Mat 1\" \"Center Mat 2\" "
    "\"Surrounding\""
    "\nAllowed options" );

  desc.add_options( )( "help,h", "produce help message" )( "version,r",
                                                           "reports comsol2aero version number" )(
    "aero-f,e",
    "output in aero-f mode instead of the default aero-s mode. Surface type (StickFixedXXX_i, "
    "SlipMovingXXX_i, InletFixedXXX_i, etc.) may be defined by using the -n [ --names ] "
    "argument." )( "matusage,m",
                   "outputs matusage directives in addition to attributes. To be used when "
                   "relevant matlaw is enabled." )(
    "selections,s",
    po::value< std::vector< std::string > >( )
      ->multitoken( )
      ->default_value( std::vector< std::string >( ) )
      ->implicit_value( std::vector< std::string >( ) ),
    "convert comsol geometry ids (domains) to selection ids. Useful in applying matlaw or matusage "
    "with fine control. Later selections overwrite earlier ones. A list of accepted selection "
    "names (labels in comsol) can be provided to restrict processing these selections." )(
    "names,n",
    po::value< std::vector< std::string > >( )->multitoken( ),
    "Generated surface name prefixes. The prefixes are used to define aero-f surface type and are "
    "not allowed in aero-s mode." )(
    "output,o",
    po::value< std::string >( ),
    "output file name. If no output argument is provided the program streams to stdout." )(
    "force,f",
    "force verbose output when output file name argument is not provided. Allowed only in verbose "
    "mode." )( "verbose,v",
               "verbose mode. Messages are streamed to std::clog (and hence stderr)." );

  tri_t   triv;
  auto    texttr = triv.helpText( );
  tet_t   tetv;
  auto    textt = tetv.helpText( );
  pyr_t   pyrv;
  auto    textp = pyrv.helpText( );
  prism_t prismv;
  auto    textpr = prismv.helpText( );
  hex_t   hexv;
  auto    texth = hexv.helpText( );

  po::options_description mappings( "Element mapping (numbers indicate aero element type)" );
  mappings.add_options( )(
    "tri", po::value< tri_t >( &triv )->default_value( 3 ), texttr.c_str( ) )(
    "tet", po::value< tet_t >( &tetv )->default_value( 23 ), textt.c_str( ) )(
    "pyr", po::value< pyr_t >( &pyrv )->default_value( 17 ), textp.c_str( ) )(
    "prism", po::value< prism_t >( &prismv )->default_value( 24 ), textpr.c_str( ) )(
    "hex", po::value< hex_t >( &hexv )->default_value( 17 ), texth.c_str( ) );

  desc.add( mappings );
  po::positional_options_description p;
  p.add( "input", -1 );

  po::options_description hidden( "Hidden options" );
  hidden.add_options( )( "input,i", po::value< std::string >( ), "input file name" );

  po::options_description all;
  all.add( desc ).add( hidden );

  po::variables_map vm;
  po::store( po::command_line_parser( ac, av ).options( all ).positional( p ).run( ), vm );
  po::notify( vm );

  if ( vm.count( "verbose" ) )
  {
    options.verbose = true;
  }

  if ( vm.count( "aero-f" ) )
  {
    options.aerof = true;
  }

  if ( vm.count( "selections" ) )
  {
    options.use_selections = true;
  }

  options.accepted_selections = vm[ "selections" ].as< std::vector< std::string > >( );

  if ( !vm[ "names" ].empty( ) )
  {

    if ( options.aerof == false )
    {
      throw std::invalid_argument( "-n [ --names ] option is allowed only aero-f mode." );
    }
    options.surface_name_prefixes = vm[ "names" ].as< std::vector< std::string > >( );
  }

  if ( vm.count( "force" ) )
  {
    if ( !options.verbose )
    {
      throw std::invalid_argument(
        "-f [ --force ] option is allowed only if -v [ --verbose ] is present." );
    }
    options.force = true;
  }

  if ( vm.count( "matusage" ) )
  {
    options.matusage = true;
  }

  char_streamer< std::ostream > stdclog( std::clog, options.verbose );

  stdclog.print( "Comsol to Aero v.", VERSION, ". Built: ", __TIME__, ", ", __DATE__ );

  if ( vm.count( "version" ) )
  {
    options.version_request = true;
    cout << "Comsol to Aero v." << VERSION << ". Built: " << __TIME__ << ", " << __DATE__
         << std::endl;
    return options;
  }

  if ( vm.count( "help" ) )
  {
    options.help = true;

    cout << desc << "\n---\n";

    cout << "comsol2aero\n";
    cout << '\n';
    cout << "AUTHORIZATION TO USE AND DISTRIBUTE\n";
    cout << '\n';
    cout << "By using or distributing the comsol2aero software (\"THE SOFTWARE\"), you agree to the following terms\n";
    cout << "governing the use and redistribution of THE SOFTWARE originally developed at the U.S. Naval\n";
    cout << "Research Laboratory (NRL), Computational Multiphysics SystemS Lab., Code 6394.\n";
    cout << '\n';
    cout << "The modules of comsol2aero containing an attribution in their header files to the NRL have been\n";
    cout << "authored by federal employees. To the extent that a federal employee is an author of a portion of\n";
    cout << "this software or a derivative work thereof, no copyright is claimed by the United States\n";
    cout << "Government, as represented by the Secretary of the Navy (\"GOVERNMENT\") under Title 17, U.S. Code.\n";
    cout << "All Other Rights Reserved. To the extent that a non-federal employee is an author of a portion of\n";
    cout << "this software or a derivative work thereof, the work was funded in whole or in part by the U.S.\n";
    cout << "Government, and is, therefore, subject to the following license: The Government is granted for\n";
    cout << "itself and others acting on its behalf a paid-up, nonexclusive, irrevocable worldwide license in\n";
    cout << "this work to reproduce, prepare derivative works, distribute copies to the public, and perform\n";
    cout << "publicly and display publicly, by or on behalf of the Government. All other rights are reserved\n";
    cout << "by the copyright owner.\n";
    cout << '\n';
    cout << "Redistribution and use in source and binary forms, with or without modification, are permitted\n";
    cout << "provided that the following conditions are met:\n";
    cout << '\n';
    cout << "(1) source code distributions retain the above copyright notice, this list of conditions, and the\n";
    cout << "following disclaimer in its entirety,\n";
    cout << "(2) distributions including binary code include this paragraph in its entirety in the\n";
    cout << "documentation or other materials provided with the distribution, and\n";
    cout << "(3) all published research using this software display the following acknowledgment:\n";
    cout << "\"This work uses the software components contained within the NRL comsol2aero computer package written\n";
    cout << "and developed by the U.S. Naval Research Laboratory, Computational Multiphysics Systems lab.,\n";
    cout << "Code 6394\"\n";
    cout << '\n';
    cout << "Neither the name of NRL or its contributors, nor any entity of the United States Government may\n";
    cout << "be used to endorse or promote products derived from this software, nor does the inclusion of the\n";
    cout << "NRL written and developed software directly or indirectly suggest NRL's or the United States\n";
    cout << "Government's endorsement of this product.\n";
    cout << '\n';
    cout << "THE SOFTWARE IS PROVIDED\"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT\n";
    cout << "NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n";
    cout << "NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR THE U.S. GOVERNMENT BE LIABLE FOR ANY CLAIM,\n";
    cout << "DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n";
    cout << "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n";
    cout << '\n';
    cout << "Notice of Third-Party Software Licenses\n";
    cout << '\n';
    cout << "This software contains open source software packages from third parties. These are available on\n";
    cout << "an \"as is\" basis and subject to their individual license agreements.\n";

    return options;
  }

  bool piped = ( !is_stdin_a_tty( ) );

  if ( vm.count( "input" ) )
  {
    if ( piped )
    {
      throw std::invalid_argument(
        "You can either redirect from standrad input or a file or provide an input file name not "
        "combinations of those." );
    }
    options.inputFileName = vm[ "input" ].as< string >( );
  }
  else
  {
    if ( !piped )
    {
      throw std::invalid_argument(
        "You must either specify the input filename, or redirect from standrad input or a file." );
    }
  }

  if ( vm.count( "output" ) )
  {
    options.outputFileName = vm[ "output" ].as< string >( );

    if ( options.force )
    {
      throw std::invalid_argument(
        "-f [ --force ] option has no effect and is not allowed when the output target is a "
        "file." );
    }
  }
  else if ( options.verbose && !options.force )
  {
    throw std::invalid_argument(
      "When an output filename is not specified run with -f to force verbose output to stderr." );
  }

  options.elementMapping[ "tri" ] = triv.value;
  options.elementMapping[ "tet" ] = tetv.value;
  // options.elementMapping[ "tet_50_96_103" ] = tetv.value;
  options.elementMapping[ "pyr" ]   = pyrv.value;
  options.elementMapping[ "prism" ] = prismv.value;
  options.elementMapping[ "hex" ]   = hexv.value;

  return options;
}
