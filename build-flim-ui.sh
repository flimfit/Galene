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
export CC=/usr/local/opt/llvm/bin/clang
export CXX=/usr/local/opt/llvm/bin/clang++
export LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib"
export PATH="/usr/local/opt/qt5/bin:$PATH"
export MACOSX_DEPLOYMENT_TARGET=10.10

cmake -GNinja -HSource/ome-cmake-superbuild -Bome-files-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=ome-files-install -Dsource-cache="cache/source/" -Dhead:BOOL=ON -Dtest:BOOL=OFF -Dextended-tests:BOOL=OFF -Dqtgui:BOOL=OFF
cmake --build ome-files-build

# Generate make files and build 
cmake -GNinja -HSource -BBuild -DCMAKE_BUILD_TYPE=Release
cmake --build Build

# sign code (requires that signature is installed in keychain)
codesign --verbose --deep -s P6MM899VL9 Build/flim-ui/Galene.app/
