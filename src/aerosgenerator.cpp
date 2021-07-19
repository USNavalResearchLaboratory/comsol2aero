// comsol2aero: a simple comsol mesh to frg aero mesh converter

#include "aerosgenerator.hpp"
#include "utils.hpp"
#include <iomanip>
#include <memory>
#include <set>

namespace aeros
{

generator::generator( bool verb, bool matusage, const mesh_t& aMesh ) :
  mesh( aMesh ), stdclog( clog, verb ), debugstdout( cerr, true ), matusage_( matusage )
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
