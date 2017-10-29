#!/bin/bash

# Build script for Galene on Mac. 
# Requires: 
#    - MacOS >=10.12
#    - CMake >= 3.7
#    - Qt >=5.7
# Currently assumes these are installed by brew, i.e. using 
#    - brew install qt cmake

# Set manual locations for clang, Qt. 
# Override these if not installed via brew
export OME_FILES_ROOT=$(pwd)/ome-files-install
export PATH="$OME_FILES_ROOT/lib:/usr/local/opt/qt5/bin:$PATH"
export DYLIB_LIBRARY_PATH="$OME_FILES_ROOT/lib"
export CMAKE_PREFIX_PATH=$(pwd)/lib-install

#brew install --with-clang llvm qt cmake opencv boost xerces-c xalan-c dlib

if [ ! -d "ome-files-install" ]; then
   cmake -GNinja -Home-cmake-superbuild -Bome-files-build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=ome-files-install \
      -Dsource-cache="cache/source/" \
      -Dhead:BOOL=ON \
      -Dtest:BOOL=OFF \
      -Dextended-tests:BOOL=OFF \
      -Dqtgui:BOOL=OFF \
      -Dbuild-prerequisites:BOOL=OFF ^
      -Dbuild-packages=ome-common;ome-model;ome-files ^
   cmake --build ome-files-build --target install
fi

# Generate make files and build 
BUILD_TYPE=Debug
#rm -rf Build_$BUILD_TYPE
cmake -GNinja -HSource -BBuild_$BUILD_TYPE -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build Build_$BUILD_TYPE  # --clean-first

# sign code (requires that signature is installed in keychain)
# codesign --verbose --deep -s P6MM899VL9 Build/flim-ui/Galene.app/
