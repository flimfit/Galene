
cmake_minimum_required(VERSION 3.7)
project(ThirdParty)

include(ExternalProject)
ExternalProject_Add(
   bioimageconvert
   GIT_REPOSITORY "https://github.com/seanwarren/bioimageconvert.git"
   GIT_TAG "master"
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/bioimageconvert"
   CMAKE_ARGS  -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/install 
               -DBIC_ENABLE_FFMPEG=OFF 
               -DBIC_ENABLE_GDCM=OFF 
               -DBIC_ENABLE_GDCM=OFF 
               -DBIC_ENABLE_JXRLIB=OFF 
               -DBIC_ENABLE_LIBJPEG_TURBO=OFF 
               -DBIC_ENABLE_LIBWEBP=OFF 
               -DBIC_ENABLE_NITFI=OFF 
               -DBIC_ENABLE_QT=OFF 
               -DBIC_ENABLE_OPENCV=ON 
               -DBIC_ENABLE_OPENMP=OFF 
               -DBIC_ENABLE_IMGCNV=OFF 
               -DLIBBIOIMAGE_TRANSFORMS=ON
               -DBIC_INTERNAL_EXIV2=OFF
   TEST_COMMAND ""
 )

 ExternalProject_Add(
   ome-cmake-superbuild
   GIT_REPOSITORY "https://github.com/seanwarren/ome-cmake-superbuild.git"
   GIT_TAG "master"
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/ome-cmake-superbuild"
   CMAKE_ARGS  -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/install 
               -Dsource-cache="lib/cache/source/" 
               -Dhead:BOOL=ON 
               -Dtest:BOOL=OFF 
               -Dextended-tests:BOOL=OFF 
               -Dqtgui:BOOL=OFF 
               -Dbuild-prerequisites:BOOL=OFF 
               -Dbuild-packages=ome-common$<SEMICOLON>ome-model$<SEMICOLON>ome-files
   TEST_COMMAND ""
 )

 ExternalProject_Add(
   dlib
   GIT_REPOSITORY "https://github.com/seanwarren/dlib.git"
   GIT_TAG "master"
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/dlib"
   CMAKE_ARGS  -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/install 
               -DDLIB_USE_BLAS=ON 
               -DDLIB_USE_LAPACK=ON 
               -DDLIB_USE_CUDA=OFF 
               -DDLIB_JPEG_SUPPORT=OFF 
               -DDLIB_LINK_WITH_SQLITE3=OFF 
               -DDLIB_PNG_SUPPORT=OFF 
               -DDLIB_GIF_SUPPORT=OFF 
               -DDLIB_NO_GUI_SUPPORT=ON 
   TEST_COMMAND ""
 )


