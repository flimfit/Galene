
cmake_minimum_required(VERSION 3.7)
project(ThirdParty)

include(ExternalProject)

set(EXT_PROJ_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/install;
                  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE};
                  -DCMAKE_DEBUG_POSTFIX=d; 
                  -DCMAKE_RELWITHDEBINFO_POSTFIX=rd;
                  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})

message(${EXT_PROJ_ARGS})


ExternalProject_Add(
  bioimageconvert
  GIT_REPOSITORY "https://github.com/seanwarren/bioimageconvert.git"
  GIT_TAG "master"
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/bioimageconvert"
  STAMP_DIR "${CMAKE_SOURCE_DIR}/stamp/bioimageconvert"
  BUILD_ALWAYS OFF
  CMAKE_ARGS  -DBIC_ENABLE_FFMPEG=OFF 
              -DBIC_ENABLE_GDCM=OFF 
              -DBIC_ENABLE_GDCM=OFF 
              -DBIC_ENABLE_JXRLIB=OFF 
              -DBIC_ENABLE_LIBJPEG_TURBO=OFF
              -DBIC_INTERNAL_LIBJPEG=ON
              -DBIC_ENABLE_LIBWEBP=OFF 
              -DBIC_ENABLE_NIFTI=OFF 
              -DBIC_ENABLE_QT=OFF 
              -DBIC_ENABLE_OPENCV=ON
              -DBIC_ENABLE_OPENMP=OFF 
              -DBIC_ENABLE_IMGCNV=OFF 
              -DLIBBIOIMAGE_TRANSFORMS=OFF
              ${EXT_PROJ_ARGS}
  TEST_COMMAND ""
)

ExternalProject_Add(
  dlib
  GIT_REPOSITORY "https://github.com/seanwarren/dlib.git"
  GIT_TAG "master"
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/dlib"
  STAMP_DIR "${CMAKE_SOURCE_DIR}/stamp/dlib"
  BUILD_ALWAYS ON
  CMAKE_ARGS  -DDLIB_USE_BLAS=ON 
              -DDLIB_USE_LAPACK=ON 
              -DDLIB_USE_CUDA=OFF 
              -DDLIB_JPEG_SUPPORT=OFF 
              -DDLIB_LINK_WITH_SQLITE3=OFF 
              -DDLIB_PNG_SUPPORT=OFF 
              -DDLIB_GIF_SUPPORT=OFF 
              -DDLIB_NO_GUI_SUPPORT=ON 
              ${EXT_PROJ_ARGS}               
  TEST_COMMAND ""
)

ExternalProject_Add(
   ome-cmake-superbuild
   GIT_REPOSITORY "https://github.com/seanwarren/ome-cmake-superbuild.git"
   GIT_TAG "master"
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/ome-cmake-superbuild"
   STAMP_DIR "${CMAKE_SOURCE_DIR}/stamp/ome-cmake-superbuild"  
   CMAKE_ARGS  -Dsource-cache=${CMAKE_SOURCE_DIR}/cache/source/ 
               -Dhead:BOOL=ON 
               -Dtest:BOOL=OFF 
               -Dextended-tests:BOOL=OFF 
               -Dqtgui:BOOL=OFF 
               -Dbuild-prerequisites:BOOL=OFF 
               -Dbuild-packages=ome-common$<SEMICOLON>ome-model$<SEMICOLON>ome-files
               ${EXT_PROJ_ARGS}               
   TEST_COMMAND ""
)


