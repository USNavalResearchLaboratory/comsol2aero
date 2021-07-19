#include "aerofgenerator.hpp"
#include "utils.hpp"
#include <iomanip>
#include <memory>
#include <set>

namespace aerof
{

generator::generator( bool verb, const mesh_t& aMesh ) :
  mesh( aMesh ), stdclog( clog, verb ), debugstdout( cerr, true )
{
}

void generator::generate( string fileName ) const
{

  stdclog.print( "\nOpening for aero mesh output: ", fileName, "\n" );

  ofstream file;
  file.open( fileName );

  if ( !file )
  {
    stringstream ss;
    ss << "Could not open file " << fileName << " for writing.";

    throw runtime_error( ss.str( ) );
  }

  generate( file );
}

} // namespace aerogenerator
