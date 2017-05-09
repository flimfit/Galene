#!/bin/bash

export CC=/usr/local/opt/llvm/bin/clang
export CXX=/usr/local/opt/llvm/bin/clang++
export LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib -L/usr/local/opt/opencv3/lib"
export OSX_SDK_VERSION=10.12

#export CC=/usr/local/bin/gcc-6
#export CXX=/usr/local/bin/g++-6
#export MACOSX_DEPLOYMENT_TARGET=10.10.0
export PATH="/usr/local/opt/qt5/bin:$PATH"

export MACOSX_DEPLOYMENT_TARGET=10.12

cmake -G"Unix Makefiles" -HSource -BBuild

cmake --build Build
