// comsol2aero: a simple comsol mesh to frg aero mesh converter

#ifndef UTILS_HPP
#define UTILS_HPP
template< class S, typename T >
void safe_print( S& stream, const T& value )
{
  stream << value;
}

template< class S, typename T, typename... Args >
void safe_print( S& stream, const T& value, Args... args )
{
  stream << value;
  safe_print( stream, args... );
}

#endif // UTILS_HPP
