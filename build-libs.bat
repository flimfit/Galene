@echo off

REM =====================================
REM Install the following packages wth vcpkg and chocolatey before running this script:
REM vcpkg install opencv:x64-windows fftw3:x64-windows boost:x64-windows xalan-c:x64-windows exiv2:x64-windows libraw:x64-windows eigen3:x64-windows proj4:x64-windows
REM choco install cmake python2 pip nsis -y
REM pip install genshi 
REM =====================================

REM Execute this command first to switch to VS2017 console
REM "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

SET PATH=%PATH%;%VCPKG_ROOT%\installed\x64-windows\bin;%VCPKG_ROOT%\installed\x64-windows\debug\bin
SET TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
SET TOOLCHAIN_FILE=%TOOLCHAIN_FILE:\=/%



SET BIOIMAGE_BUILD_FLAGS=-DCMAKE_INSTALL_PREFIX=lib/install ^
   -DBIC_ENABLE_FFMPEG=OFF ^
   -DBIC_ENABLE_GDCM=OFF ^
   -DBIC_ENABLE_JXRLIB=OFF ^
   -DBIC_ENABLE_LIBJPEG_TURBO=OFF ^
   -DBIC_ENABLE_LIBWEBP=OFF ^
   -DBIC_ENABLE_NITFI=OFF ^
   -DBIC_ENABLE_QT=OFF ^
   -DBIC_ENABLE_OPENCV=ON ^
   -DBIC_ENABLE_OPENMP=OFF ^
   -DBIC_ENABLE_IMGCNV=OFF ^
   -DLIBBIOIMAGE_TRANSFORMS=OFF ^
   -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE% -DVCPKG_TARGET_TRIPLET=x64-windows

REM build release
cmake -GNinja -Hlib/src/bioimageconvert -Blib/build/bioimageconvert/release -DCMAKE_BUILD_TYPE=Release %BIOIMAGE_BUILD_FLAGS%
cmake --build lib/build/bioimageconvert/release --target install
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

REM build debug
cmake -GNinja -Hlib/src/bioimageconvert -Blib/build/bioimageconvert/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_DEBUG_POSTFIX=d %BIOIMAGE_BUILD_FLAGS%
cmake --build lib/build/bioimageconvert/debug --target install
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

REM build relwithdebinfo
cmake -GNinja -Hlib/src/bioimageconvert -Blib/build/bioimageconvert/relwithdebinfo -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_RELWITHDEBINFO_POSTFIX=rd %BIOIMAGE_BUILD_FLAGS%
cmake --build lib/build/bioimageconvert/relwithdebinfo --target install
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%



SET OME_BUILD_FLAGS=-DCMAKE_INSTALL_PREFIX=lib/install/ome-files ^
   -Dsource-cache="lib/cache/source/" ^
   -Dhead:BOOL=ON ^
   -Dtest:BOOL=OFF ^
   -Dextended-tests:BOOL=OFF ^
   -Dbuild-prerequisites:BOOL=OFF ^
   -Dbuild-packages=ome-common;ome-model;ome-files ^
   -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE% -DVCPKG_TARGET_TRIPLET=x64-windows

REM build release
cmake -GNinja -Hlib/src/ome-cmake-superbuild -Blib/build/ome-files/release -DCMAKE_BUILD_TYPE=Release %OME_BUILD_FLAGS%
cmake --build lib/build/ome-files/release --target install
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

REM build debug
cmake -GNinja -Hlib/src/ome-cmake-superbuild -Blib/build/ome-files/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_DEBUG_POSTFIX=d %OME_BUILD_FLAGS%
cmake --build lib/build/ome-files/debug --target install
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

REM build relwithdebinfo
cmake -GNinja -Hlib/src/ome-cmake-superbuild -Blib/build/ome-files/relwithdebinfo -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_RELWITHDEBINFO_POSTFIX=rd %OME_BUILD_FLAGS%
cmake --build lib/build/ome-files/relwithdebinfo --target install
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
