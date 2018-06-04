
cmake_minimum_required(VERSION 3.7)
project(ThirdParty)

include(ExternalProject)

set(EXT_PROJ_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/install;
                  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE};
                  -DCMAKE_DEBUG_POSTFIX=d; 
                  -DCMAKE_RELWITHDEBINFO_POSTFIX=rd;
                  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})

ExternalProject_Add(
  bioimageconvert
  HG_REPOSITORY "https://bitbucket.org/dimin/bioimageconvert/src"
  HG_TAG "469d87c15a5b24c4881046fa68a43b12acc25639"
  UPDATE_COMMAND ""
  PATCH_COMMAND hg revert --all && hg import ${CMAKE_SOURCE_DIR}/bioimageconvert.patch --partial --no-commit
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/bioimageconvert"
  STAMP_DIR "${CMAKE_SOURCE_DIR}/stamp/bioimageconvert"
  BUILD_ALWAYS ON
  CMAKE_ARGS  -DBIC_ENABLE_FFMPEG=OFF 
              -DBIC_ENABLE_GDCM=OFF 
              -DBIC_ENABLE_JXRLIB=OFF 
              -DBIC_ENABLE_LIBJPEG_TURBO=OFF
              -DBIC_INTERNAL_LIBJPEG=OFF
              -DBIC_INTERNAL_EXIV2=OFF
              -DBIC_ENABLE_LIBWEBP=OFF 
              -DBIC_ENABLE_NIFTI=OFF 
              -DBIC_ENABLE_QT=OFF 
              -DBIC_ENABLE_OPENCV=ON
              -DBIC_ENABLE_OPENMP=OFF 
              -DBIC_ENABLE_IMGCNV=OFF 
              -DLIBBIOIMAGE_TRANSFORMS=OFF
              -DZLIB_ROOT=${ZLIB_ROOT}
              ${EXT_PROJ_ARGS}
  TEST_COMMAND ""
)

ExternalProject_Add(
   ome-cmake-superbuild
   GIT_REPOSITORY "https://github.com/ome/ome-cmake-superbuild.git"
   GIT_TAG "9cc778639549dccdb5542740b2db895f9a5b1a12"
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


