// comsol2aero: a simple comsol mesh to frg aero mesh converter

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
