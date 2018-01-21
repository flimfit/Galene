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
export OME_FILES_ROOT=$(pwd)/lib/install
export PATH="$OME_FILES_ROOT/lib:/usr/local/opt/qt5/bin:$PATH"
export DYLIB_LIBRARY_PATH="$OME_FILES_ROOT/lib"
export CMAKE_PREFIX_PATH=$(pwd)/lib/install

# Generate make files and build 
BUILD_TYPE=Release
#rm -rf Build/$BUILD_TYPE
cmake -GNinja -HSource -BBuild/$BUILD_TYPE -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DOME_FILES_ROOT="$OME_FILES_ROOT" 
cmake --build Build/$BUILD_TYPE  # --clean-first

# sign code (requires that signature is installed in keychain)
# codesign --verbose --deep -s P6MM899VL9 Build/$BUILD_TYPE/flim-ui/Galene.app/
