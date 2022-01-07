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

inline const auto comment = ( "#" >> lexeme[ +( char_ - eol ) ] );

template< typename Iterator >
struct MeshSkipper : public grammar< Iterator >
{
  MeshSkipper( ) : MeshSkipper::base_type( skip )
  {
    skip = +( space | comment );
  }

  rule< Iterator > skip;
};

// Connectivity data grammar
template< typename Iterator, class skipper = MeshSkipper< Iterator > >
struct ElementSetGrammar : grammar< Iterator, ElementSet( ), locals< size_t, size_t >, skipper >
{
  ElementSetGrammar( ) : ElementSetGrammar::base_type( set, "Comsol Element Set" )
  {
    // FIXME: Maybe relax the element type requirements here.
    element_type %= lexeme[ uint_ > +space ]
                    > ( qi::string( "vtx" ) | qi::string( "edg" ) | qi::string( "tet" )
                        | qi::string( "tri" ) | qi::string( "quad" ) | qi::string( "hex" )
                        | qi::string( "pyr" ) | qi::string( "prism" ) );

    geom_indicies_count %= omit[ uint_( ref( elemCount ) ) ];
    geom_indicies_count.name( "geometric indicies count equal to element count" );

    set
      %= element_type > omit[ uint_[ _a = _1 ] ]                 // Number of nodes per element
         > omit[ uint_[ ref( elemCount ) = _1 ] ]                // Number of elements
         > repeat( ref( elemCount ) )[ repeat( _a )[ double_ ] ] // Elements
         > geom_indicies_count // Number of geometric indicies: must be equal to number of elements
         > repeat( ref( elemCount ) )[ uint_ ]; // Geometric Indicies // FIXME: Handle error on
                                                // repetition properly

    set.name( "Comsol element set definition" );
  }

  rule< Iterator, ElementSet( ), locals< size_t, size_t >, skipper > set;

  rule< Iterator, skipper > geom_indicies_count;

  rule< Iterator, ElementSet::ElementType( ), skipper > element_type;

  size_t elemCount = 0;
};

// Mesh (nodes + connectivity)
template< typename Iterator, class skipper = MeshSkipper< Iterator > >
struct MeshObjectGrammar : grammar< Iterator, MeshObject( ), locals< size_t, size_t >, skipper >
{
  MeshObjectGrammar( ) : MeshObjectGrammar::base_type( object, "Comsol mesh object" )
  {

    baseIndex %= uint_( 0 );
    baseIndex.name( "lowest mesh point index equal to 0" );

    element_sets
      %= omit[ uint_[ _a = _1 ] ] > repeat(
           _a )[ elem_parser ]; // Fixme: enforce that number of element sets later in parsing
    element_sets.name( "Element sets" );

    point %= repeat( ref( sdim ) )[ double_ ];
    point.name( "Point coordinates" );

    coords %= repeat( ref( numPoints ) )[ point ];
    coords.name( "Mesh points definition" );

    object
      %= omit[ uint_ > uint_
               > uint_ ] // Not sure about what these three numbers are in the comsol mesh file
         > lexeme[ uint_ > +space > lit( "Mesh" ) ] // Fixme: Currently we support only mesh objects
         > uint_                                    // Version (Comsol version?)
         > uint_[ ref( sdim ) = _1 ]                // Number of space dimensions
         > uint_[ ref( numPoints ) = _1 ]           // Number of points
         > baseIndex     // First index. FIXME: Support non 0 base indexing
         > coords        // Point coordinates
         > element_sets; // Element Sets
  }

  rule< Iterator, MeshObject( ), locals< size_t, size_t >, skipper >      object;
  rule< Iterator, MeshObject::ElementSets( ), locals< size_t >, skipper > element_sets;
  rule< Iterator, size_t( ), skipper >                                    baseIndex;
  rule< Iterator, MeshObject::Point( ), skipper >                         point;
  rule< Iterator, MeshObject::Coords( ), skipper >                        coords;

  ElementSetGrammar< Iterator > elem_parser;

  size_t sdim = 0;

  size_t numPoints = 0;
};

// Selection (nodes + connectivity)
template< typename Iterator, class skipper = MeshSkipper< Iterator > >
struct SelectionObjectGrammar :
  grammar< Iterator, SelectionObject( ), locals< size_t, size_t >, skipper >
{
  SelectionObjectGrammar( ) : SelectionObjectGrammar::base_type( object, "Comsol selection object" )
  {

    label %= omit[ uint_ ] > lexeme[ +( char_ - eol - comment ) ];
    label.name( "Object label followed by # Label" );
    //  baseIndex.name( "lowest selection point index equal to 0" );

    entities %= repeat( ref( numEntities ) )[ uint_ ];
    entities.name( "Selection entities" );

    object %= omit[ uint_ > uint_
                    > uint_ ] // Not sure about what those three numbers are in the comsol mesh file
              > lexeme[ uint_ > +space > lit( "Selection" ) ] > uint_ > label
              > omit[ uint_ > lexeme[ +( char_ - eol - comment ) ] ] > uint_ // # Dimension
              > omit[ uint_[ ref( numEntities ) = _1 ] ]                     // # Number of entities
              > entities;
  }

  rule< Iterator, SelectionObject( ), locals< size_t, size_t >, skipper > object;
  rule< Iterator, string( ), skipper >                                    label;
  rule< Iterator, SelectionObject::Entities( ), skipper >                 entities;

  size_t numEntities = 0;
};

template< typename Iterator, class skipper = MeshSkipper< Iterator > >
struct MeshGrammar : grammar< Iterator, Mesh( ), skipper >
{
  MeshGrammar( ErrorHandler< Iterator >& error_handler ) :
    MeshGrammar::base_type( mesh, "Comsol mesh" ), error_handler_( error_handler )
  {
    typedef function< ErrorHandler< Iterator > > ErrorHandler_function;

    timestamp %= no_skip[ lit( "# Created by COMSOL Multiphysics" ) >> +( char_ - eol ) ];
    timestamp.name( "Timestamp preceeded by \"# Created by COMSOL Multiphysics\"" );

    version %= uint_ > uint_;
    version.name( "Version id in the form: integer integer" );

    tags %= omit[ uint_[ _a = _1 ] ]
            > repeat( _a )[ lexeme[ uint_ > +space ] > lexeme[ +( char_ - eol - comment ) ] ];
    tags.name( "Tags definition" );

    types %= omit[ uint_[ _a = _1 ] ] > repeat(
               _a )[ lexeme[ uint_ > +space ]
                     > lexeme[ +( char_ - eol - comment ) ] ]; // TODO: I think all theses should be
                                                               // +( char_ - eol) - comment
    types.name( "Object types definition" );

    comsol_mesh_object %= obj_parser;
    comsol_mesh_object.name( "mesh object" );

    comsol_selection_object %= sel_obj_parser;
    comsol_selection_object.name( "selection object" );

    mesh %= no_skip[ eps ] > timestamp > version > tags > types > comsol_mesh_object
            > repeat[ comsol_selection_object ];

    on_error< fail >( mesh, ErrorHandler_function( error_handler_ )( "Error:", _4, _3 ) );
  }

  rule< Iterator, Mesh( ), skipper >                          mesh;
  rule< Iterator, string( ), skipper >                        timestamp;
  rule< Iterator, Mesh::Version( ), skipper >                 version;
  rule< Iterator, Mesh::Tags( ), locals< size_t >, skipper >  tags;
  rule< Iterator, Mesh::Types( ), locals< size_t >, skipper > types;
  rule< Iterator, MeshObject( ), skipper >                    comsol_mesh_object;
  rule< Iterator, SelectionObject( ), skipper >               comsol_selection_object;

  MeshObjectGrammar< Iterator >      obj_parser;
  SelectionObjectGrammar< Iterator > sel_obj_parser;

  ErrorHandler< Iterator >& error_handler_;
};

class Parser
{
public:
  Parser( bool verb );

  template< class S >
  void parse( S& stream )
  {

    model = Mesh( );

    string storage;

    stream.unsetf( ios::skipws ); // No white space skipping

    copy(
      istream_iterator< char >( stream ), istream_iterator< char >( ), back_inserter( storage ) );

    string::const_iterator iter = storage.begin( );
    string::const_iterator end  = storage.end( );

    ErrorHandler< string::const_iterator > error_handler( iter, end );

    typedef MeshGrammar< string::const_iterator > grammar;

    grammar mesh_parser( error_handler );

    typedef MeshSkipper< string::const_iterator > skipper_type;

    skipper_type skipper;

    bool r = phrase_parse( iter, end, mesh_parser, skipper, model );

    if ( r && iter == end )
    {
      // Todo, can we move the trimming inside the parsing?
      for ( auto& selection_objects : model.selection_object )
      {
        selection_objects.label = trim( selection_objects.label );
      }
      print_model( );
    }
    else
    {
      throw runtime_error( "Parsing failed" );
    }
  }

  void parse( string& file_name );

  const Mesh& getModel( ) const
  {
    return model;
  }

private:
  void print_model( );

  Mesh model;

  CharStreamer< ostream > stdclog;
#ifdef NDEBUG
  NoneCharStreamer< ostream > debugstdout;
#else
  char_streamer< ostream > debugstdout;
#endif
};

}

#endif // COMSOLPARSER_H
