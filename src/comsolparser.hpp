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

#ifndef COMSOLPARSER_HPP
#define COMSOLPARSER_HPP

#include "charstreamer.hpp"
#include "comsolmesh.hpp"
#include "parsingerrorhandler.hpp"

#include <boost/config/warning_disable.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_lexeme.hpp>

std::string trim( const std::string& str );

namespace comsol
{

namespace spirit  = boost::spirit;
namespace phoenix = boost::phoenix;
namespace ascii   = boost::spirit::ascii;
namespace qi      = boost::spirit::qi;

using phoenix::construct;
using phoenix::function;
using phoenix::ref;
using phoenix::val;

using ascii::char_;
using spirit::omit;
using spirit::repeat;

using namespace qi;

using std::string;

template< typename Iterator >
struct mesh_skipper : public grammar< Iterator >
{
  mesh_skipper( ) : mesh_skipper::base_type( skip )
  {
    skip = +( space | ( "#" >> lexeme[ +( char_ - eol ) ] ) );
  }

  rule< Iterator > skip;
};

// Connectivity data grammar
template< typename Iterator, class skipper = mesh_skipper< Iterator > >
struct element_set_grammar : grammar< Iterator, element_set( ), locals< size_t, size_t >, skipper >
{
  element_set_grammar( ) : element_set_grammar::base_type( set, "Comsol Element Set" )
  {
    // FIXME: Maybe relax the element type requirements here.
    elementType %= lexeme[ uint_ > +space ]
                   > ( qi::string( "vtx" ) | qi::string( "edg" ) | qi::string( "tet" )
                       | qi::string( "tri" ) | qi::string( "quad" ) | qi::string( "hex" )
                       | qi::string( "pyr" ) | qi::string( "prism" ) );

    geomIndiciesCount %= omit[ uint_( ref( elemCount ) ) ];
    geomIndiciesCount.name( "geometric indicies count equal to element count" );

    set %= elementType > omit[ uint_[ _a = _1 ] ]                  // Number of nodes per element
           > omit[ uint_[ ref( elemCount ) = _1 ] ]                // Number of elements
           > repeat( ref( elemCount ) )[ repeat( _a )[ double_ ] ] // Elements
           > geomIndiciesCount // Number of geometric indicies: must be equal to number of elements
           > repeat( ref( elemCount ) )[ uint_ ]; // Geometric Indicies // FIXME: Handle error on
                                                  // repetition properly

    set.name( "Comsol element set definition" );
  }

  rule< Iterator, element_set( ), locals< size_t, size_t >, skipper > set;

  rule< Iterator, skipper > geomIndiciesCount;

  rule< Iterator, element_set::element_type( ), skipper > elementType;

  size_t elemCount = 0;
};

// Mesh (nodes + connectivity)
template< typename Iterator, class skipper = mesh_skipper< Iterator > >
struct mesh_object_grammar : grammar< Iterator, mesh_object( ), locals< size_t, size_t >, skipper >
{
  mesh_object_grammar( ) : mesh_object_grammar::base_type( object, "Comsol mesh object" )
  {

    baseIndex %= uint_( 0 );
    baseIndex.name( "lowest mesh point index equal to 0" );

    elementSets
      %= omit[ uint_[ _a = _1 ] ] > repeat(
           _a )[ elemParser ]; // Fixme: enforce that number of element sets later in parsing
    elementSets.name( "Element sets" );

    point %= repeat( ref( sdim ) )[ double_ ];
    point.name( "Point coordinates" );

    coords %= repeat( ref( numPoints ) )[ point ];
    coords.name( "Mesh points definition" );

    object
      %= omit[ uint_ > uint_
               > uint_ ] // Not sure about what those three numbers are in the comsol mesh file
         > lexeme[ uint_ > +space > lit( "Mesh" ) ] // Fixme: Currently we support only mesh objects
         > uint_                                    // Version (Comsol version?)
         > uint_[ ref( sdim ) = _1 ]                // Number of space dimensions
         > uint_[ ref( numPoints ) = _1 ]           // Number of points
         > baseIndex    // First index. FIXME: Support non 0 base indexing
         > coords       // Point coordinates
         > elementSets; // Element Sets
  }

  rule< Iterator, mesh_object( ), locals< size_t, size_t >, skipper >       object;
  rule< Iterator, mesh_object::element_sets( ), locals< size_t >, skipper > elementSets;
  rule< Iterator, size_t( ), skipper >                                      baseIndex;
  rule< Iterator, mesh_object::point_t( ), skipper >                        point;
  rule< Iterator, mesh_object::coords_t( ), skipper >                       coords;

  element_set_grammar< Iterator > elemParser;

  size_t sdim = 0;

  size_t numPoints = 0;
};

// Selection (nodes + connectivity)
template< typename Iterator, class skipper = mesh_skipper< Iterator > >
struct selection_object_grammar :
  grammar< Iterator, selection_object_t( ), locals< size_t, size_t >, skipper >
{
  selection_object_grammar( ) :
    selection_object_grammar::base_type( object, "Comsol selection object" )
  {

    label %= omit[ uint_ ] > lexeme[ +( char_ - eol - ( "#" >> lexeme[ +( char_ - eol ) ] ) ) ];
    label.name( "Object label followed by # Label" );
    //  baseIndex.name( "lowest selection point index equal to 0" );

    entities %= repeat( ref( numEntities ) )[ uint_ ];
    entities.name( "Selection entities" );

    object %= omit[ uint_ > uint_
                    > uint_ ] // Not sure about what those three numbers are in the comsol mesh file
              > lexeme[ uint_ > +space
                        > lit( "Selection" ) ] // Fixme: Currently we support only mesh objects
              > uint_                          // Version (Comsol version?)
              > label
              > omit[ uint_ > lexeme[ +( char_ - eol - ( "#" >> lexeme[ +( char_ - eol ) ] ) ) ] ]
              > omit[ lexeme[ +( char_ - eol ) ] ] > omit[ uint_[ ref( numEntities ) = _1 ] ]
              > entities;
  }

  rule< Iterator, selection_object_t( ), locals< size_t, size_t >, skipper > object;
  rule< Iterator, string( ), skipper >                                       label;
  rule< Iterator, selection_object_t::entities_t( ), skipper >               entities;

  size_t numEntities = 0;
};

template< typename Iterator, class skipper = mesh_skipper< Iterator > >
struct mesh_grammar : grammar< Iterator, mesh_t( ), skipper >
{
  mesh_grammar( error_handler< Iterator >& errorHandler ) :
    mesh_grammar::base_type( mesh, "Comsol mesh" ), errorHandler_( errorHandler )
  {
    typedef function< error_handler< Iterator > > error_handler_function;

    timestamp %= no_skip[ lit( "# Created by COMSOL Multiphysics" ) >> +( char_ - eol ) ];
    timestamp.name( "Timestamp preceeded by \"# Created by COMSOL Multiphysics\"" );

    version %= uint_ > uint_;
    version.name( "Version id in the form: integer integer" );

    tags %= omit[ uint_[ _a = _1 ] ]
            > repeat( _a )[ lexeme[ uint_ > +space ]
                            > lexeme[ +( char_ - eol - ( "#" >> lexeme[ +( char_ - eol ) ] ) ) ] ];
    tags.name( "Tags definition" );

    types %= omit[ uint_[ _a = _1 ] ]
             > repeat( _a )[ lexeme[ uint_ > +space ]
                             > lexeme[ +( char_ - eol - ( "#" >> lexeme[ +( char_ - eol ) ] ) ) ] ];
    types.name( "Object types definition" );

    comsolMeshObject %= objParser;
    comsolMeshObject.name( "mesh object" );

    comsolSelectionObject %= selObjParser;
    comsolSelectionObject.name( "selection object" );

    mesh %= no_skip[ eps ] > timestamp > version > tags > types > comsolMeshObject
            > repeat[ comsolSelectionObject ];

    on_error< fail >( mesh, error_handler_function( errorHandler_ )( "Error:", _4, _3 ) );
  }

  rule< Iterator, mesh_t( ), skipper >                            mesh;
  rule< Iterator, string( ), skipper >                            timestamp;
  rule< Iterator, mesh_t::version_t( ), skipper >                 version;
  rule< Iterator, mesh_t::tags_t( ), locals< size_t >, skipper >  tags;
  rule< Iterator, mesh_t::types_t( ), locals< size_t >, skipper > types;
  rule< Iterator, mesh_object( ), skipper >                       comsolMeshObject;
  rule< Iterator, selection_object_t( ), skipper >                comsolSelectionObject;

  mesh_object_grammar< Iterator >      objParser;
  selection_object_grammar< Iterator > selObjParser;

  error_handler< Iterator >& errorHandler_;
};

class parser
{
public:
  parser( bool verb );

  template< class S >
  void parse( S& stream )
  {

    model = mesh_t( );

    string storage;

    stream.unsetf( ios::skipws ); // No white space skipping

    copy(
      istream_iterator< char >( stream ), istream_iterator< char >( ), back_inserter( storage ) );

    string::const_iterator iter = storage.begin( );
    string::const_iterator end  = storage.end( );

    error_handler< string::const_iterator > errorHandler( iter, end );

    typedef mesh_grammar< string::const_iterator > grammar;

    grammar meshParser( errorHandler );

    typedef mesh_skipper< string::const_iterator > skipper_type;

    skipper_type skipper;

    bool r = phrase_parse( iter, end, meshParser, skipper, model );

    if ( r && iter == end )
    {
      // Todo, can we move the trimming inside the parsing?
      for ( auto& selectionObjects : model.selection_objects )
      {
        selectionObjects.label = trim( selectionObjects.label );
      }
      printModel( );
    }
    else
    {
      throw runtime_error( "Parsing failed" );
    }
  }

  void parse( string& fileName );

  const mesh_t& getModel( ) const
  {
    return model;
  }

private:
  void printModel( );

  mesh_t model;

  char_streamer< ostream > stdclog;
#ifdef NDEBUG
  none_char_streamer< ostream > debugstdout;
#else
  char_streamer< ostream > debugstdout;
#endif
};

}

#endif // COMSOLPARSER_H
