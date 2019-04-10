@echo off

SETLOCAL

SET VSCOMMUNITYCMD="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
SET VSBUILDCMD="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat"

:: Set up Visual Studio environment variables
IF EXIST %VSCOMMUNITYCMD% CALL %VSCOMMUNITYCMD% -arch=amd64 && GOTO :BUILD
IF EXIST %VSBUILDCMD% CALL %VSBUILDCMD% -arch=amd64 && GOTO :BUILD
ECHO Error: Visual Studio install not found && EXIT /B 1

:BUILD
SET TRIPLET=x64-windows-static
SET PATH=%PATH%;%VCPKG_ROOT%\installed\%TRIPLET%\bin;%VCPKG_ROOT%\installed\%TRIPLET%\debug\bin
SET TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
SET TOOLCHAIN_FILE=%TOOLCHAIN_FILE:\=/%
SET OME_FILES_ROOT=%VCPKG_ROOT%\installed\%TRIPLET%


SET BFTOOLS_DIR=%cd%\lib\bftools
SET BFTOOLS_DIR=%BFTOOLS_DIR:\=/%

IF "%1"=="--clean" rmdir Build /s /q

echo Generating CMake Project
echo Using Generator: "Visual Studio 15 2017 Win64"
cmake -G "Visual Studio 15 2017 Win64" -HSource -BBuild -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%" -DOME_FILES_ROOT="%OME_FILES_ROOT%" -DVCPKG_TARGET_TRIPLET=%TRIPLET%
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

echo Building 64bit Project in Release mode
cmake --build Build --config Release --target PACKAGE
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
