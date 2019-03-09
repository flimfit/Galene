#!/bin/bash

export VCPKGROOT=~/vcpkg

# use brew zlib for compatability with macOS 10.11 (provides older zlib)
cmake -GNinja -Hlib -Blib/build -DCMAKE_TOOLCHAIN_FILE=$VCPKGROOT/scripts/buildsystems/vcpkg.cmake 
cmake --build lib/build 