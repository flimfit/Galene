#!/bin/bash

# Build script for Galene on Mac. 
# Requires: 
#    - MacOS >=10.12
#    - Qt >=5.7
#    - Up-to-date LLVM (i.e. not supplied by Apple)
# Currently assumes these are installed by brew, i.e. using 
#    - brew install qt5 llvm

# Set manual locations for clang, Qt. 
# Override these if not installed via brew
export CC=/usr/local/opt/llvm/bin/clang
export CXX=/usr/local/opt/llvm/bin/clang++
export LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib"
export PATH="/usr/local/opt/qt5/bin:$PATH"

# Set deployment target
export OSX_SDK_VERSION=10.12
export MACOSX_DEPLOYMENT_TARGET=10.12

# Generate make files and build 
cmake -G"Unix Makefiles" -HSource -BBuild
cmake --build Build -- -j

# fix up the half-ass job done by macdeployqt
# see: https://github.com/iltommi/macdeployqtfix/blob/master/macdeployqtfix.py
python macdeployqtfix.py Build/flim-ui/flim_ui.app/Contents/MacOS/flim_ui /usr/local/opt/qt5

# sign code (requires that signature is installed in keychain)
codesign -s P6MM899VL9 Build/flim-ui/flim_ui.app/
