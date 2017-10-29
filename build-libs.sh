#!/bin/bash


# Generate make files and build 
BUILD_TYPE=Debug
BUILD_DIR=lib-build/bioimageconvert_$BUILD_TYPE
rm -rf $BUILD_DIR
cmake -GNinja -Hlib/bioimageconvert -B$BUILD_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
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
   -DBIC_ENABLE_IMGCNV=ON \
   -DLIBBIOIMAGE_TRANSFORMS=ON \
   -DCMAKE_INSTALL_PREFIX=lib-install

cmake --build $BUILD_DIR --target install # --clean-first