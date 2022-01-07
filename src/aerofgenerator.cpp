#include "aerofgenerator.hpp"
#include "utils.hpp"
#include <iomanip>
#include <memory>
#include <set>

namespace aerof
{

Generator::Generator( bool verb, const Mesh& aero_mesh ) :
  mesh( aero_mesh ), stdclog( clog, verb ), debugstdout( cerr, true )
{
}

void Generator::generate( string file_name ) const
{

  stdclog.print( "\nOpening for aero mesh output: ", file_name, "\n" );

  ofstream file;
  file.open( file_name );

  if ( !file )
  {
    stringstream ss;
    ss << "Could not open file " << file_name << " for writing.";

    throw runtime_error( ss.str( ) );
  }

  generate( file );
}

} // namespace aerogenerator
