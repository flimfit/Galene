#!/bin/bash

# Build script for Galene on Mac. 
# Requires: 
#    - MacOS >=10.12
#    - CMake >= 3.7

VCPKGROOT=~/vcpkg
DYLIB_LIBRARY_PATH="$OME_FILES_ROOT/lib"
export OME_FILES_ROOT=$VCPKG_ROOT/installed/x64-osx

# Generate make files and build 
BUILD_TYPE=Release
[ "$1" == "--clean" ] && rm -rf Build/$BUILD_TYPE
cmake -GNinja -HSource -BBuild/$BUILD_TYPE \
   -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
   -DOME_FILES_ROOT="$OME_FILES_ROOT" \
   -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake 

cmake --build Build/$BUILD_TYPE  # --clean-first
