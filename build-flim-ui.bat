@echo off

IF NOT DEFINED MSVC_VER SET MSVC_VER=15
if %MSVC_VER%==13 SET MSVC_YEAR=2013
if %MSVC_VER%==14 SET MSVC_YEAR=2015
if %MSVC_VER%==15 SET MSVC_YEAR=2017

REM echo Cleaning CMake Project
REM SET PROJECT_DIR=Build\
REM rmdir %PROJECT_DIR% /s /q
REM mkdir %PROJECT_DIR%

if %MSVC_VER%==15 (set GENERATOR="Visual Studio %MSVC_VER% Win64"
) else set GENERATOR="Visual Studio %MSVC_VER% %MSVC_YEAR% Win64"
echo Generating CMake Project in: %PROJECT_DIR%
echo Using Generator: %GENERATOR%
cmake -G %GENERATOR% -HSource -BBuild

echo Building 64bit Project in Release mode
cmake --build Build --config Release --target PACKAGE
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
