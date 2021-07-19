// comsol2aero: a simple comsol mesh to frg aero mesh converter

#ifndef char_streamer_H
#define char_streamer_H

#include "utils.hpp"

template< class S >
class char_streamer
{
public:
  char_streamer( S& stream, bool active = true ) : stream_( stream ), active_( active )
  {
  }

  void setActive( bool active )
  {
    active_ = active;
  }

  template< typename... Args >
  void print( Args... args ) const
  {
    if ( active_ )
    {
      safe_print( stream_, args..., '\n' );
    }
  }

  const bool& isActive( ) const
  {
    return active_;
  }

private:
  S&   stream_;
  bool active_;
};

// For NDEBUG
template< class S >
class none_char_streamer
{
public:
  none_char_streamer( S& /*stream*/, bool /*active = true*/ )
  {
  }

  template< typename... Args >
  void print( Args... /*args*/ )
  {
  }

  bool isActive( )
    const // Fixme::make this constexpr (MSVC doesn't support currently ( May 2014 ) )
  {
    return false;
  }
};

#endif // char_streamer_H
