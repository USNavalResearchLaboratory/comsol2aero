# comsol2aero
comsol2areo is a mesh converter tool that takes as inputs comsol mphtxt files and exports them to [aero-s](https://bitbucket.org/frg/aero-f) and [aero-f](https://bitbucket.org/frg/aero-f) mesh files . 
## Compilation
The source code depends on boost spirit library, boost program_options and is tested using v. 1.61 of boost. ([www.boost.org](www.boost.org)).

The build process is based on cmake. A suggested compilation sequence is:
 
```
# 1. Download comsol2aero source code and place it in a folder. For example $HOME/source/comsol2aero
# 2. Create a build directory:
mkdir -p $HOME/builds/comsol2aero 
# 3. Run cmake
cd  $HOME/builds/comsol2aero
cmake $HOME/source/comsol2aero
# 4. Build comsol2aero
make -j 4
# 5. The comsol2aero executable is placed in the $HOME/builds/comsol2aero folder
```
The default compilation process builds a static executable that is compatible with a very large number of linux flavors and has been tested to work with distributions dating back 10 years (as of May 2017).

Compiling on windows may require to manually specify the boost root dir. For example in the cmake command line (or your build tool) you may want to include something like: -DBOOST_ROOT=C:/local/boost_1_76_0

## Documentation
The comsol2aero documentation can be displayed by executing:
```
comsol2aero --help
```
Example meshes are located in the ```examples``` folder.
