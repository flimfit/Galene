#!/bin/bash

# Build BioImage
#========================
BUILD_DIR=lib-build/bioimageconvert
#rm -rf $BUILD_DIR
cmake -GNinja -Hlib/bioimageconvert -B$BUILD_DIR -DCMAKE_BUILD_TYPE=Release \
   -DBIC_ENABLE_FFMPEG=OFF \
   -DBIC_ENABLE_GDCM=OFF \
   -DBIC_ENABLE_GDCM=OFF \
   -DBIC_ENABLE_JXRLIB=OFF \
   -DBIC_ENABLE_LIBJPEG_TURBO=OFF \
   -DBIC_ENABLE_LIBWEBP=OFF \
   -DBIC_ENABLE_NITFI=OFF \
   -DBIC_ENABLE_QT=OFF \
   -DBIC_ENABLE_OPENCV=ON \
   -DBIC_ENABLE_OPENMP=OFF \
   -DBIC_ENABLE_IMGCNV=OFF \
   -DLIBBIOIMAGE_TRANSFORMS=ON \
   -DCMAKE_INSTALL_PREFIX=lib-install

cmake --build $BUILD_DIR --target install

# Build ome-files
#========================
cmake -GNinja -Hlib/ome-cmake-superbuild -Blib-build/ome-files -DCMAKE_BUILD_TYPE=Release \
   -DCMAKE_INSTALL_PREFIX=lib-install \
   -Dsource-cache="lib/cache/source/" \
   -Dhead:BOOL=ON \
   -Dtest:BOOL=OFF \
   -Dextended-tests:BOOL=OFF \
   -Dqtgui:BOOL=OFF \
   -Dbuild-prerequisites:BOOL=OFF \
   -Dbuild-packages=ome-common\;ome-model\;ome-files

cmake --build lib-build/ome-files --target install
