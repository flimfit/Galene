#!/bin/bash

# Build script for Galene on Mac. 
# Requires: 
#    - MacOS >=10.12
#    - CMake >= 3.7
#    - Qt >=5.7
# Currently assumes these are installed by brew, i.e. using 
#    - brew install qt cmake opencv boost xerces-c xalan-c exiv2 libraw

# Set manual locations for clang, Qt. 
# Override these if not installed via brew
VCPKGROOT=~/vcpkg
DYLIB_LIBRARY_PATH="$OME_FILES_ROOT/lib"
CMAKE_PREFIX_PATH=$(pwd)/lib/install
export OME_FILES_ROOT=$VCPKG_ROOT/installed/x64-osx
export BFTOOLS_DIR=$(pwd)/lib/bftools

# Generate make files and build 
BUILD_TYPE=Release
[ "$1" == "--clean" ] && rm -rf Build/$BUILD_TYPE
cmake -GNinja -HSource -BBuild/$BUILD_TYPE \
   -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
   -DOME_FILES_ROOT="$OME_FILES_ROOT" \
   -DCMAKE_TOOLCHAIN_FILE=$VCPKGROOT/scripts/buildsystems/vcpkg.cmake 

cmake --build Build/$BUILD_TYPE  # --clean-first
