#!/bin/bash

# Build script for Galene on Mac. 
# Requires: 
#    - MacOS >=10.10
#    - CMake >= 3.7
#    - Qt >=5.7
#    - Up-to-date LLVM (i.e. not supplied by Apple)
# Currently assumes these are installed by brew, i.e. using 
#    - brew install --with-clang llvm qt cmake

# Set manual locations for clang, Qt. 
# Override these if not installed via brew
export OME_FILES_ROOT=$(pwd)/ome-files-install
#export CC=/usr/local/opt/llvm/bin/clang
#export CXX=/usr/local/opt/llvm/bin/clang++
#export LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib"
export PATH="$OME_FILES_ROOT/lib:/usr/local/opt/qt5/bin:$PATH"
#export MACOSX_DEPLOYMENT_TARGET=10.10
export DYLIB_LIBRARY_PATH="$OME_FILES_ROOT/lib"

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
#rm -rf Build
BUILD_TYPE=Debug
cmake -G"Unix Makefiles" -HSource -BBuild -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build Build  # --clean-firstx

# sign code (requires that signature is installed in keychain)
#codesign --verbose --deep -s P6MM899VL9 Build/flim-ui/Galene.app/
