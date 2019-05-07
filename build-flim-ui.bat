@echo off
:: Build Galene package
:: 
:: Options
::    --clean            Clean build directory before build
::    --generator <gen>  Use specified generator (default is Visual Studio 15 2017 Win64)

SETLOCAL

SET VSCOMMUNITYCMD="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
SET VSBUILDCMD="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat"
SET VSENTERPRISECMD="C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"

:: Set up Visual Studio environment variables
IF EXIST %VSCOMMUNITYCMD% CALL %VSCOMMUNITYCMD% -arch=amd64 && GOTO :BUILD
IF EXIST %VSBUILDCMD% CALL %VSBUILDCMD% -arch=amd64 && GOTO :BUILD
IF EXIST %VSENTERPRISECMD% CALL %VSENTERPRISECMD% -arch=amd64 && GOTO :BUILD
ECHO Error: Visual Studio install not found && EXIT /B 1

:BUILD
SET TRIPLET=x64-windows-static
SET PATH=%PATH%;%VCPKG_ROOT%\installed\%TRIPLET%\bin;%VCPKG_ROOT%\installed\%TRIPLET%\debug\bin
SET TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
SET TOOLCHAIN_FILE=%TOOLCHAIN_FILE:\=/%
SET OME_FILES_ROOT=%VCPKG_ROOT%\installed\%TRIPLET%

:: Override CMAKE_PREFIX_PATH
SET CMAKE_PREFIX_PATH=

SET GENERATOR="Visual Studio 15 2017 Win64"

:: Parse command line arguments
:loop
IF NOT "%1"=="" (
    IF "%1"=="--clean" (
      rmdir Build /s /q
    )
    IF "%1"=="--generator" (
        SET GENERATOR=%2
        SHIFT
    )
    SHIFT
    GOTO :loop
)

:: Check if local CUDA install exists
IF EXIST %cd%\cuda\ SET PATH=%PATH%;%CD%\cuda\nvcc\bin

echo Generating CMake Project
echo Using Generator: %GENERATOR%
cmake -G%GENERATOR% -HSource -BBuild ^
   -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%" ^
   -DOME_FILES_ROOT="%OME_FILES_ROOT%" ^
   -DVCPKG_TARGET_TRIPLET=%TRIPLET% ^
   -DCMAKE_C_COMPILER=cl.exe ^
   -DCMAKE_CXX_COMPILER=cl.exe ^
   -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

echo Building 64bit Project in Release mode
cmake --build Build --config Release --target package
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
