@echo off

IF NOT DEFINED MSVC_VER SET MSVC_VER=14
if %MSVC_VER%==14 SET MSVC_YEAR=2015

echo Cleaning CMake Project
SET PROJECT_DIR=Build\
rmdir %PROJECT_DIR% /s /q
mkdir %PROJECT_DIR%
cd %PROJECT_DIR%

set GENERATOR="Visual Studio %MSVC_VER% %MSVC_YEAR% Win64"
echo Generating CMake Project in: %PROJECT_DIR%
echo Using Generator: %GENERATOR%
cmake -G %GENERATOR% ..\Source\

echo Building 64bit Project in Release mode
cmake --build . --config Release --target PACKAGE
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
