@echo off

SETLOCAL

SET PATH=%PATH%;%VCPKG_ROOT%\installed\x64-windows\bin;%VCPKG_ROOT%\installed\x64-windows\debug\bin
SET TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
SET TOOLCHAIN_FILE=%TOOLCHAIN_FILE:\=/%

SET LIB_INSTALL_PATH=%cd%\lib\install
SET LIB_INSTALL_PATH=%LIB_INSTALL_PATH:\=/%
SET OME_FILES_ROOT=%LIB_INSTALL_PATH%
SET LIB_INSTALL_PATH=%LIB_INSTALL_PATH%;%LIB_INSTALL_PATH%/lib
SET CMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH%;%LIB_INSTALL_PATH%

SET BFTOOLS_DIR=%cd%\lib\bftools
SET BFTOOLS_DIR=%BFTOOLS_DIR:\=/%

IF "%1"=="--clean" rmdir Build /s /q

echo Generating CMake Project
echo Using Generator: "Visual Studio 15 2017 Win64"
cmake -G "Visual Studio 15 2017 Win64" -HSource -BBuild -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%" -DOME_FILES_ROOT="%OME_FILES_ROOT%"
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

echo Building 64bit Project in Release mode
cmake --build Build --config Release --target PACKAGE
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
