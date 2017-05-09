#!/bin/bash

export CC=/usr/local/opt/llvm/bin/clang
export CXX=/usr/local/opt/llvm/bin/clang++
export LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib"
export OSX_SDK_VERSION=10.12

export PATH="/usr/local/opt/qt5/bin:$PATH"

export MACOSX_DEPLOYMENT_TARGET=10.12

cmake -G"Unix Makefiles" -HSource -BBuild

cmake --build Build

# fix up the half-ass job done by macdeployqt
# see: https://github.com/iltommi/macdeployqtfix/blob/master/macdeployqtfix.py
python macdeployqtfix.py Build/flim-ui/flim_ui.app/Contents/MacOS/flim_ui /usr/local/opt/qt5