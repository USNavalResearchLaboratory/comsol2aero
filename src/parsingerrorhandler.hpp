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

#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include <iostream>
#include <sstream>
#include <vector>

namespace comsol
{

template< typename Iterator >
struct error_handler
{
  // This is not working with newer boost/gcc versions
  // template <typename, typename, typename>
  // struct result { typedef void type; };

  template< class >
  struct result
  {
    typedef void type;
  };

  // This is could work but it doesn't (assuming template<class> struct result didn't have a
  // typedef)
  // template <typename F, typename X, typename Y>
  // struct result<F(X,Y)> { typedef void type; };

  error_handler( Iterator first, Iterator last ) : first( first ), last( last )
  {
  }

  template< typename Message, typename What >
  void operator( )( Message const& message, What const& what, Iterator err_pos ) const
  {
    // FIXME: replace the conversion with boost::phoenix
    std::stringstream stream;
    stream << what;
    std::string name( stream.str( ) );

    int      line;
    Iterator line_start = get_pos( err_pos, line );
    if ( err_pos != last )
    {

      std::cerr << message << " Line " << line << ". Expected " << name
                << " at or after:" << std::endl;
      std::cerr << get_line( line_start ) << std::endl;

      for ( ; line_start != err_pos; ++line_start )
        std::cerr << ' ';

      std::cerr << '^' << std::endl;
    }
    else
    {
      std::cerr << "Unexpected end of file. ";
      std::cerr << message << " Line " << line << ". Expected" << name << "." << std::endl;
    }
  }

  Iterator get_pos( Iterator err_pos, int& line ) const
  {
    line                = 1;
    Iterator i          = first;
    Iterator line_start = first;
    while ( i != err_pos )
    {
      bool eol = false;
      if ( i != err_pos && *i == '\r' ) // CR
      {
        eol        = true;
        line_start = ++i;
      }
      if ( i != err_pos && *i == '\n' ) // LF
      {
        eol        = true;
        line_start = ++i;
      }
      if ( eol )
        ++line;
      else
        ++i;
    }

    return line_start;
  }

  std::string get_line( Iterator err_pos ) const
  {
    Iterator i = err_pos;

    // position i to the next EOL
    while ( i != last && ( *i != '\r' && *i != '\n' ) )
      ++i;

    return std::string( err_pos, i );
  }

  Iterator                first;
  Iterator                last;
  std::vector< Iterator > iters;
};

} // namespace comsolParse

#endif // ERRORHANDLER_HPP
