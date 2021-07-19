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
// Notice of Third-Party Software Licenses
//
// This software uses open source software packages from third parties. These are available on
// an "as is" basis and subject to their individual license agreements. Additional information can
// can be found in the provided "licenses" folder.
//

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
