#include "comsolparser.hpp"

#include <exception>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

std::string trim( const std::string& str )
{
  using std::string;

  size_t first = str.find_first_not_of( ' ' );
  if ( string::npos == first )
  {
    return str;
  }
  size_t last = str.find_last_not_of( ' ' );
  return str.substr( first, ( last - first + 1 ) );
}

namespace comsol
{

parser::parser( bool verb ) : stdclog( clog, verb ), debugstdout( cerr, true )
{
}

void parser::parse( string& fileName )
{
  stdclog.print( "\nOpening for parsing: ", fileName, "\n---" );

  ifstream in( fileName, ios_base::in );

  if ( !in )
  {
    stringstream ss;
    ss << "Could not open file " << fileName << " for parsing.";

    throw runtime_error( ss.str( ) );
  }
  parse( in );
}

void parser::printModel( )
{
  using namespace std;

  if ( stdclog.isActive( ) )
  {
    stdclog.print( "Comsol file created on: ", model.created );

    stdclog.print( "Version: ", model.version.first, ' ', model.version.second );

    stdclog.print( "Tags\n  Count: ", model.tags.size( ) );

    for ( size_t i = 0; i != model.tags.size( ); i++ )
    {
      stdclog.print(
        "    Tag no. ", setw( 2 ), i, ": ", model.tags[ i ].first, ", ", model.tags[ i ].second );
    }

    stdclog.print( "Types\n  Count: ", model.types.size( ) );

    for ( size_t i = 0; i != model.types.size( ); i++ )
    {
      stdclog.print( "    Type no. ",
                     setw( 2 ),
                     i,
                     ": ",
                     model.types[ i ].first,
                     ", ",
                     model.types[ i ].second );
    }

    stdclog.print( "Object: " );

    stdclog.print( "  ID: ", model.object.classId );
    stdclog.print( "  Version: ", model.object.version );
    stdclog.print( "  Space dimensions: ", model.object.spaceDimensions );
    stdclog.print( "  Number of mesh points: ", model.object.coordinates.size( ) );

    stdclog.print( "  Element types\n    Count: ", model.object.element_sets.size( ) );

    for ( size_t i = 0; i != model.object.element_sets.size( ); i++ )
    {
      stdclog.print( "    Type ",
                     i,
                     ": ",
                     model.object.element_sets[ i ].element_type.first,
                     ' ',
                     model.object.element_sets[ i ].element_type.second );
      if ( model.object.element_sets[ i ].elements.size( ) > 0 )
      {
        stdclog.print( "      Nodes per element: ",
                       model.object.element_sets[ i ].elements[ 0 ].size( ) );
        stdclog.print( "      Number of elements: ",
                       model.object.element_sets[ i ].elements.size( ) );
        stdclog.print( "      Number of geometric indicies: ",
                       model.object.element_sets[ i ].geometric_indicies.size( ) );
      }
    }

    for ( const auto& so : model.selection_objects )
    {
      stdclog.print( "Selection Object: " );
      stdclog.print( "  ID: ", so.classId );
      stdclog.print( "  Label: ", so.label );
      stdclog.print( "  Dim : ", so.dimsize );
      stdclog.print( "  Number of domains: ", so.entities.size( ) );
    }
  }
}

}

// namespace comsolParse
