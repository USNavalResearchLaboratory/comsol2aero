// comsol2aero: a comsol mesh to frg aero mesh converter

#include "aerofgenerator.hpp"
#include "aerosgenerator.hpp"
#include "cmdlineparse.hpp"
#include "comsolparser.hpp"
#include "config.hpp"
#include "converter.hpp"

#include <iostream>

using namespace std;

int main( int ac, char* av[] )
{
  try
  {

    auto options = parseCommandLineOptions( ac, av );

    if ( options.help || options.version_request )
    {
      return 0;
    }

    comsol::parser parser( options.verbose );

    if ( options.inputFileName == "" )
    {
      parser.parse( std::cin );
    }
    else
    {
      parser.parse( options.inputFileName );
    }

    aero::mesh_t aeroMesh;
    converter    conv( options.verbose,
                    options.use_selections,
                    options.elementMapping,
                    options.surface_name_prefixes,
                    options.accepted_selections );

    conv.convert( parser.getModel( ), aeroMesh );

    if ( options.aerof == false )
    {
      aeros::generator generator( options.verbose, options.matusage, aeroMesh );

      if ( options.outputFileName == "" )
      {
        generator.generate( std::cout );
      }
      else
      {
        generator.generate( options.outputFileName );
      }
    }
    else
    {
      aerof::generator generator( options.verbose, aeroMesh );

      if ( options.outputFileName == "" )
      {
        generator.generate( std::cout );
      }
      else
      {
        generator.generate( options.outputFileName );
      }
    }
  }
  catch ( exception& e )
  {
    cerr << "comsol2aero: Error: " << e.what( ) << "\n";
    cerr << "Please call with --help for a help message.\n";
    return 1;
  }
  catch ( ... )
  {
    cerr << "Exception of unknown type!\n";
    return 1;
  }

  return 0;
}
