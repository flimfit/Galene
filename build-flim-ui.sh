#!/bin/bash

# Build script for Galene on Mac. 
# Requires: 
#    - MacOS >=10.12
#    - CMake >= 3.7

# Generate make files and build 
BUILD_TYPE=Release
[ "$1" == "--clean" ] && rm -rf Build/$BUILD_TYPE
cmake -GNinja -HSource -BBuild/$BUILD_TYPE \
   -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
   -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake 

cmake --build Build/$BUILD_TYPE  # --clean-first
