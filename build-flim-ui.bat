@echo off

REM =====================================
REM Install the following packages wth vcpkg and chocolatey before running this script:
REM vcpkg install opencv:x64-windows dlib:x64-windows fftw3:x64-windows boost:x64-windows xalan-c:x64-windows 
REM choco install cmake python2 pip nsis -y
REM pip install genshi 
REM =====================================

IF NOT DEFINED MSVC_VER SET MSVC_VER=15
if %MSVC_VER%==14 SET MSVC_YEAR=2015
if %MSVC_VER%==15 SET MSVC_YEAR=2017

if %MSVC_VER%==15 (set GENERATOR="Visual Studio %MSVC_VER% Win64"
) else set GENERATOR="Visual Studio %MSVC_VER% %MSVC_YEAR% Win64"

SET TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
SET TOOLCHAIN_FILE=%TOOLCHAIN_FILE:\=/%

SET PATH=%PATH%;%VCPKG_ROOT%\installed\x64-windows\bin;%VCPKG_ROOT%\installed\x64-windows\debug\bin

IF EXIST ome-files-install\ (
   REM Execute this command first to switch to VS2017 console
   REM "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

   SET OME_BUILD_FLAGS=-DCMAKE_INSTALL_PREFIX=ome-files-install ^
      -Dsource-cache="cache/source/" ^
      -Dhead:BOOL=ON ^
      -Dtest:BOOL=OFF ^
      -Dextended-tests:BOOL=OFF ^
      -Dbuild-prerequisites:BOOL=OFF ^
      -Dbuild-packages=ome-common;ome-model;ome-files ^
      -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE% -DVCPKG_TARGET_TRIPLET=x64-windows

   REM build release
   cmake -GNinja -Home-cmake-superbuild -Bome-files-build\release -DCMAKE_BUILD_TYPE=Release %OME_BUILD_FLAGS%
   cmake --build ome-files-build\release --target install
   if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
   
   REM build debug
   REM cmake -GNinja -Home-cmake-superbuild -Bome-files-build\debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_DEBUG_POSTFIX=d %OME_BUILD_FLAGS%
   REM cmake --build ome-files-build\debug --target install
   if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

   REM build relwithdebinfo
   REM cmake -GNinja -Home-cmake-superbuild -Bome-files-build\relwithdebinfo -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_RELWITHDEBINFO_POSTFIX=rd %OME_BUILD_FLAGS%
   REM cmake --build ome-files-build\relwithdebinfo --target install
   if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

)


SET OME_FILES_ROOT=%cd%\ome-files-install
SET OME_FILES_ROOT=%OME_FILES_ROOT:\=/%


REM rmdir Build /s /q
echo Generating CMake Project
echo Using Generator: %GENERATOR%
cmake -G %GENERATOR% -HSource -BBuild -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE%
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

echo Building 64bit Project in Release mode
REM cmake --build Build --config Release --target PACKAGE
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
