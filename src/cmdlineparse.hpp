// comsol2aero: a simple comsol mesh to frg aero mesh converter

#ifndef CMDLINEPARSE_HPP
#define CMDLINEPARSE_HPP

#include <map>
#include <string>
#include <vector>

struct user_options
{
  bool                                 verbose         = false;
  bool                                 force           = false;
  bool                                 help            = false;
  bool                                 version_request = false;
  bool                                 aerof           = false;
  bool                                 matusage        = false;
  bool                                 use_selections  = false;
  std::string                          inputFileName;
  std::string                          outputFileName;
  std::map< std::string, std::size_t > elementMapping;
  std::vector< std::string >           surface_name_prefixes;
  std::vector< std::string >           accepted_selections;
};

user_options parseCommandLineOptions( int ac, char* av[] );

#endif // PARSECOMMANDLINE_HPP
