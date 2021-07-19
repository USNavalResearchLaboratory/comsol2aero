// comsol2aero: a simple comsol mesh to frg aero mesh converter

#include "detecttty.hpp"

#include <stdio.h>

#ifdef _MSC_VER // FIXME: HAS NOT BEEN TESTED

#include <io.h>

#else

#include <unistd.h>

#endif

bool is_stdin_a_tty( )
{

#ifdef _MSC_VER
  return _isatty( _fileno( stdin ) );
#else
  return isatty( fileno( stdin ) );
#endif
}
