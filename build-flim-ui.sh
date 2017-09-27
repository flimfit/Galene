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
export CC=/usr/local/opt/llvm/bin/clang
export CXX=/usr/local/opt/llvm/bin/clang++
export LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib"
export PATH="$OME_FILES_ROOT/lib:/usr/local/opt/qt5/bin:$PATH"
export MACOSX_DEPLOYMENT_TARGET=10.10
export DYLIB_LIBRARY_PATH="$OME_FILES_ROOT/lib"

#brew install --with-clang llvm qt cmake libtiff libpng opencv qt5 boost xerces-c xalan-c

if [ ! -d "ome-files-install" ]; then
   cmake -GNinja -Home-cmake-superbuild -Bome-files-build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=ome-files-install \
      -Dsource-cache="cache/source/" \
      -Dhead:BOOL=ON \
      -Dtest:BOOL=OFF \
      -Dextended-tests:BOOL=OFF \
      -Dqtgui:BOOL=OFF \
      -Dome-cmake-superbuild_USE_SYSTEM_tiff:BOOL=ON \
      -Dome-cmake-superbuild_USE_SYSTEM_png:BOOL=ON \
      -Dome-cmake-superbuild_USE_SYSTEM_bzip2:BOOL=ON \
      -Dome-cmake-superbuild_USE_SYSTEM_boost:BOOL=ON \
      -Dome-cmake-superbuild_USE_SYSTEM_xalan:BOOL=ON \
      -Dome-cmake-superbuild_USE_SYSTEM_xerces:BOOL=ON 
   cmake --build ome-files-build --target install
fi

# Generate make files and build 
rm -rf Build
cmake -GNinja -HSource -BBuild -DCMAKE_BUILD_TYPE=Debug
cmake --build Build  # --clean-first

# sign code (requires that signature is installed in keychain)
codesign --verbose --deep -s P6MM899VL9 Build/flim-ui/Galene.app/
