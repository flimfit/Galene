@echo off

REM =====================================
REM Install the following packages wth vcpkg and chocolatey before running this script:
REM vcpkg install opencv:x64-windows fftw3:x64-windows boost:x64-windows xalan-c:x64-windows exiv2:x64-windows libraw:x64-windows eigen3:x64-windows proj4:x64-windows hdf5:x64-windows
REM choco install cmake python2 pip nsis -y
REM pip install genshi 
REM =====================================

REM Execute this command first to switch to VS2017 console, otherwise Ninja gets confused
REM "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

SETLOCAL

SET PATH=%PATH%;%VCPKG_ROOT%\installed\x64-windows\bin;%VCPKG_ROOT%\installed\x64-windows\debug\bin
SET TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
SET TOOLCHAIN_FILE=%TOOLCHAIN_FILE:\=/%
SET BUILD_FLAGS=-DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE%
SET CMAKE_PREFIX_PATH=""

cmake -GNinja -Hlib -Blib/build/RelWithDebInfo -DCMAKE_BUILD_TYPE=RelWithDebInfo %BUILD_FLAGS%
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
cmake --build lib/build/RelWithDebInfo
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

cmake -GNinja -Hlib -Blib/build/Release -DCMAKE_BUILD_TYPE=Release %BUILD_FLAGS%
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
cmake --build lib/build/Release
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

cmake -GNinja -Hlib -Blib/build/Debug -DCMAKE_BUILD_TYPE=Debug %BUILD_FLAGS%
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
cmake --build lib/build/Debug
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

