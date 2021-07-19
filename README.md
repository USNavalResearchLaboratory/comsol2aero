# comsol2aero
comsol2areo is a mesh converter tool that takes as inputs comsol mphtxt files and exports them to [aero-s](https://bitbucket.org/frg/aero-f) and [aero-f](https://bitbucket.org/frg/aero-f) mesh files . 
## Compilation
The source code depends on the boost spirit library, boost program_options and is tested to work using at least v. 1.61 of boost. ([www.boost.org](www.boost.org)). To compile you will need:
1. A modern c++ compiler (Tested using GCC on Linux and MSVC on Windows)
2. Boost > v1.61
3. cmake

A suggested compilation sequence is:
### Linux 
```
# 1. Install a compiler, cmake and boost for example in Ubuntu:
sudo apt install build-essential libboost-all-dev cmake
# 2. Clone the repository
git clone git@github.com:USNavalResearchLaboratory/comsol2aero.git
# 2. Change to the repository directory and configure:
cd comsol2aero
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
# 3. Build:
cmake --build . -j4 --config Release
```
The default compilation process builds a static executable that is compatible with a very large number of linux flavors and has been tested to work with distributions dating back 10 years.

### Windows 
```
# 1. Install MS Visual Studio or mingw and cmake.
# 2. Clone the repository
git clone git@github.com:USNavalResearchLaboratory/comsol2aero.git
# 2. Change to the repository directory and configure:
cd comsol2aero
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBOOST_ROOT=C:/local/boost_1_76_0 # Your boost directory may be at a different location
# 3. Build:
cmake --build . -j4 --config Release
```

## Documentation
The comsol2aero documentation and examples command line arguments can be displayed by executing:
```
comsol2aero --help
```
The example meshes are located in the ```examples``` folder.
