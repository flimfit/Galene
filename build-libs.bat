@echo off

REM =====================================
REM Install the following packages wth vcpkg and chocolatey before running this script:
REM vcpkg install opencv:x64-windows fftw3:x64-windows boost:x64-windows xalan-c:x64-windows exiv2:x64-windows libraw:x64-windows eigen3:x64-windows proj4:x64-windows hdf5:x64-windows
REM choco install cmake python2 pip nsis ninja -y
REM pip install genshi 
REM =====================================

SETLOCAL

SET VSCOMMUNITYCMD="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
SET VSBUILDCMD="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat"

:: Set up Visual Studio environment variables
IF EXIST %VSCOMMUNITYCMD% CALL %VSCOMMUNITYCMD% -arch=amd64 && GOTO :BUILD
IF EXIST %VSBUILDCMD% CALL %VSBUILDCMD% -arch=amd64 && GOTO :BUILD
ECHO Error: Visual Studio install not found && EXIT /B 1

:BUILD
SET TRIPLET=x64-windows
SET PATH=%PATH%;%VCPKG_ROOT%\installed\%TRIPLET%\bin;%VCPKG_ROOT%\installed\%TRIPLET%\debug\bin
SET TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
SET TOOLCHAIN_FILE=%TOOLCHAIN_FILE:\=/%
SET BUILD_FLAGS=-DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE%
SET CMAKE_PREFIX_PATH=""

mkdir lib\install

IF NOT "%1"=="--build-debug" GOTO build_release

cmake -GNinja -Hlib -Blib/build/RelWithDebInfo -DCMAKE_BUILD_TYPE=RelWithDebInfo %BUILD_FLAGS%
ECHO %ERRORLEVEL%
IF %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
cmake --build lib/build/RelWithDebInfo
IF %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

cmake -GNinja -Hlib -Blib/build/Debug -DCMAKE_BUILD_TYPE=Debug %BUILD_FLAGS%
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
cmake --build lib/build/Debug
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

:build_release

cmake -GNinja -Hlib -Blib/build/Release -DCMAKE_BUILD_TYPE=Release %BUILD_FLAGS%
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
cmake --build lib/build/Release -- -j 2
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%


