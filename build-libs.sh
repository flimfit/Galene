#!/bin/bash

cmake -GNinja -Hlib -Blib/build
cmake --build lib/build