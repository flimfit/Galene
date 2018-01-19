#!/bin/bash

# use brew zlib for compatability with macOS 10.11 (provides older zlib)
cmake -GNinja -Hlib -Blib/build -DZLIB_ROOT=/usr/local/opt/zlib
cmake --build lib/build