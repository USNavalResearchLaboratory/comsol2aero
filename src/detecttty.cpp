#include "detecttty.hpp"

#include <stdio.h>

#ifdef _MSC_VER // FIXME: HAS NOT BEEN TESTED

#include <io.h>

#else

#include <unistd.h>

#endif

bool IsStdinAtty( )
{

#ifdef _MSC_VER
  return _isatty( _fileno( stdin ) );
#else
  return isatty( fileno( stdin ) );
#endif
}
