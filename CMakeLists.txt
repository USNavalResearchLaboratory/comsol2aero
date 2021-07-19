# comsol2aero: a simple comsol mesh to frg aero mesh converter
cmake_minimum_required( VERSION 3.19 )

project( comsol2aero )
include( cmake/dependentoption.cmake )
unset  ( QT_QMAKE_EXECUTABLE ) # Silence Qt creator

# BUILD TYPE ----------------
if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release CACHE STRING
        "Select the build type. Options are: None, Debug, Release,
        RelWithDebInfo, MinSizeRel." FORCE )
endif()

# LINKING TYPE ----------------
option( COMSOL2AERO_LINK_ALL_STATIC "Build with full static linkage. Useful for
    using with other distributions (includes libc, libstdc++...)" ON)

cmake_dependent_option( COMSOL2AERO_LINK_DEPS_STATIC "Links to available static
    versions of dependent libs." ON "NOT COMSOL2AERO_LINK_ALL_STATIC" ON)

if( COMSOL2AERO_LINK_DEPS_STATIC )
    # Workaround: For some reason if this is enabled with
    # COMSOL2AERO_LINK_ALL_STATIC, then libc is getting dynamically linked
    set(Boost_USE_STATIC_LIBS   ON)
    set(Boost_USE_STATIC_RUNTIME ON)
endif()

file(GLOB SRC_LIST "src/*.hpp" "src/*.cpp")
add_executable( comsol2aero ${SRC_LIST} )
set_property(TARGET comsol2aero PROPERTY CXX_STANDARD 17)
install( TARGETS comsol2aero RUNTIME DESTINATION bin)

set_target_properties(comsol2aero PROPERTIES COMPILE_DEFINITIONS BUILDER_STATIC_DEFINE)

if ( NOT CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions ( comsol2aero PRIVATE  NDEBUG )
endif()

# DEPENDENCIES ----------------
find_package( Boost REQUIRED COMPONENTS program_options)

if(NOT TARGET Boost::program_options)
    add_library(Boost::program_options IMPORTED INTERFACE)
    set_property(TARGET Boost::program_options PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
    set_property(TARGET Boost::program_options PROPERTY
        INTERFACE_LINK_LIBRARIES ${Boost_LIBRARIES})
endif()

target_compile_definitions (
    comsol2aero PRIVATE
    BOOST_PHOENIX_NO_VARIADIC_EXPRESSION
    BOOST_PHOENIX_NO_VARIADIC_FUNCTION_EVAL
    BOOST_RESULT_OF_USE_TR1
)

target_link_libraries( comsol2aero Boost::program_options )

# FLAGS ---------------------------------------------------------
if( (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") )
    if( COMSOL2AERO_LINK_ALL_STATIC )
        target_compile_options( comsol2aero PUBLIC /MT )
        target_link_options( comsol2aero PUBLIC /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT )
    endif()
endif()

if( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
    if(NOT CMAKE_BUILD_TYPE MATCHES Debug AND NOT CMAKE_BUILD_TYPE MATCHES None)
        # Optimize for size
        target_compile_options( comsol2aero PRIVATE "-s" )
        set(CMAKE_EXE_LINKER_FLAGS "-s") # TODO: introduce target_link_options
                                         # for appropriate cmake version.
                                         # I think >=3.13
    endif()

    if( COMSOL2AERO_LINK_ALL_STATIC )
        set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static" )
    endif()
endif()

message( "comsol2aero data folder (use to run examples):\n " ${CMAKE_CURRENT_SOURCE_DIR}/examples "\n" )

# Ubuntu Sizes and Execution timings:
# static stripped( 1.95MB)                           time: 0.002s
# static stripped + upx compressed (690kB).          time: 0.016s

# shared stripped( 367)                              time: 0.001s - 0.002s
# shared stripped + upx compressed (93kB).           time: 0.002s

# Shared core libs striped (650kB)                   time: 0.001s - 0.002s
# Shared core libs striped + upx compressed.(174kB)  time: 0.003s - 0.006s
