###############################################################################
## project configuration ######################################################
###############################################################################

cmake_minimum_required( VERSION 3.7 )
list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )

set( AUTHOR "Patrick Lindemann <patrick.lindemann.99@gmail.com>" )
project( warzone-osm-mapmaker VERSION 1.0 )

set( CMAKE_CXX_STANDARD 17 )

###############################################################################
## program options ############################################################
###############################################################################

# Compile options that will be added to the generated compile_commands.json file
# in the build directory. This is needed to make code linting with clangd work
# within visual studio code.
if ( UNIX )
    add_compile_options( -I/usr/include/c++/9 -I/usr/include/x86_64-linux-gnu/c++/9 )
endif()

###############################################################################
## file globbing ##############################################################
###############################################################################

# These instructions search the directory tree when cmake is
# invoked and put all files that match the pattern in the variables 
# `sources` and `data`.
file( GLOB_RECURSE sources      src/main/*.cpp src/main/*.hpp )
file( GLOB_RECURSE sources_test src/test/*.cpp )
file( GLOB_RECURSE data resources/* )
# You can use set( sources src/main.cpp ) etc if you don't want to
# use globbing to find files automatically.

###############################################################################
## target definitions #########################################################
###############################################################################

# The data is just added to the executable, because in some IDEs ( QtCreator ) 
# files are invisible when they are not explicitly part of the project.
add_executable( ${PROJECT_NAME} ${sources} ${data} )

# This allows to include files relative to the root of the src directory with a <> pair
target_include_directories( ${PROJECT_NAME} PUBLIC src/main )

# Include directiories from the include folder
# include_directories( ${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/include )

# This copies all resource files in the build directory.
# We need this, because we want to work with paths relative to the executable.
file( COPY ${data} DESTINATION resources )

###############################################################################
## external project configuration #############################################
###############################################################################

# Include the ExternalProject module to fetch dependencies during the build process.
include( ExternalProject )

# Set the protocol for git repository downloads.
# If you build this project behind a firewall, you might change this to "http".
set ( GIT_PROTOCOL "git" )

# Set
set( VENDOR_CMAKE_DIR ${CMAKE_SOURCE_DIR}/cmake/vendor )
set( VENDOR_BUILD_DIR ${CMAKE_BINARY_DIR}/vendor )

###############################################################################
## pre-requisite dependencies #################################################
###############################################################################

# Git is a free and open source distributed version control system designed to handle everything
# from small to very large projects with speed and efficiency.
# https://git-scm.com/
find_package( Git REQUIRED )

# BOOST provides free peer-reviewed portable C++ source libraries.
# https://www.boost.org/
find_package( Boost REQUIRED COMPONENTS filesystem program_options )

# THREADS is needed for parallelized program execution.
if ( WIN32 )
    set ( CMAKE_USE_WIN32_THREADS_INIT ON )
endif()
set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )

# Include the pre-installed directories
target_include_directories( ${PROJECT_NAME} PUBLIC
    ${Boost_INCLUDE_DIR}
)

# Include the pre-installed libraries
target_link_libraries( ${PROJECT_NAME} PUBLIC
    ${Boost_LIBRARIES}
    Threads::Threads
)

###############################################################################
## dependencies ###############################################################
###############################################################################

# BZIP2 is a freely available, patent free high-quality data
# compressor.
# https://www.sourceware.org/bzip2/
# This is needed to work with compressed input and output (.bz2) files.
include( ${VENDOR_CMAKE_DIR}/External-BZip2.cmake )

# ZLIB is a massively spiffy yet delicately unobtrusive compression library.
# https://zlib.net/
# This is needed to work with protocol buffer format (.pbf) and other compressed
# input and output files.
include ( ${VENDOR_CMAKE_DIR}/External-ZLib.cmake )

# PROTOZERO is a minimalistic protocol buffer decoder and encoder in C++.
# https://github.com/mapbox/protozero
include ( ${VENDOR_CMAKE_DIR}/External-Protozero.cmake )

# EXPAT is a a stream-oriented XML parser library written in C.
# https://libexpat.github.io/
# This is needed to work with common OSM XML file formats such as .osm files.
include ( ${VENDOR_CMAKE_DIR}/External-Expat.cmake )

# RAPIDJSON is a fast JSON parser/generator for C++ with both SAX/DOM style API.
# https://rapidjson.org/
include( ${VENDOR_CMAKE_DIR}/External-Rapidjson.cmake )

# OSMIUM is a fast and flexible C++ library for working with OpenStreetMap data.
# https://osmcode.org/libosmium/
include( ${VENDOR_CMAKE_DIR}/External-Libosmium.cmake )

# Declare the dependencies
add_dependencies( ${PROJECT_NAME}
    bzip2
    zlib
    protozero
    expat
    rapidjson
    libosmium
)

# Include the built header-only and other depencies.
target_include_directories( ${PROJECT_NAME} PUBLIC
    ${BZIP2_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${PROTOZERO_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
    ${OSMIUM_INCLUDE_DIR}
)

# Link the built libraries
# target_link_libraries( ${PROJECT_NAME} PUBLIC -static-${ZLIB_LIBRARIES})
target_link_libraries( ${PROJECT_NAME} PUBLIC
    ${BZIP2_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${EXPAT_LIBRARIES}
)

###############################################################################
## testing ####################################################################
###############################################################################

# This is for our testing framework,
# we don't add REQUIRED because it's just for testing.
# People who might want to build the project to use it should not be required
# to install testing dependencies.
# find_package( GTest )

if( GTEST_FOUND )
  add_executable( unit_tests ${sources_test} ${sources} )

  # This define is added to prevent collision with the main.
  # It might be better solved by not adding the source with the main to the
  # testing target.
  target_compile_definitions( unit_tests PUBLIC UNIT_TESTS )

  # This allows us to use the executable as a link library, and inherit all 
  # linker options and library dependencies from it, by simply adding it as dependency.
  set_target_properties( ${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS on )

  target_link_libraries( unit_tests PUBLIC
    ${GTEST_BOTH_LIBRARIES}
    ${PROJECT_NAME}
  )

  target_include_directories( unit_tests PUBLIC
    ${GTEST_INCLUDE_DIRS} # doesn't do anything on linux
  )
  
endif()

###############################################################################
## packaging ##################################################################
###############################################################################

# All install commands get the same destination. this allows us to use paths
# relative to the executable.
install( TARGETS ${PROJECT_NAME} DESTINATION example_destination )

# This is basically a repeat of the file copy instruction that copies the
# resources in the build directory, but here we tell cmake that we want it
# in the package.
install( DIRECTORY resources DESTINATION example_destination )

# Now comes everything we need, to create a package
# there are a lot more variables you can set, and some
# you need to set for some package types, but we want to
# be minimal here.
set( CPACK_PACKAGE_NAME ${PROJECT_NAME} )
set( CPACK_PACKAGE_VERSION ${PROJECT_VERSION} )

# We don't want to split our program up into several incomplete pieces.
set( CPACK_MONOLITHIC_INSTALL 1 )

# This must be last
include( CPack )