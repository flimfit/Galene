@echo off

SETLOCAL

SET PATH=%PATH%;%VCPKG_ROOT%\installed\x64-windows\bin;%VCPKG_ROOT%\installed\x64-windows\debug\bin
SET TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
SET TOOLCHAIN_FILE=%TOOLCHAIN_FILE:\=/%

SET CMAKE_PREFIX_PATH=%cd%\lib\install
SET CMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH:\=/%
SET OME_FILES_ROOT=%CMAKE_PREFIX_PATH%

REM rmdir Build /s /q
echo Generating CMake Project
echo Using Generator: "Visual Studio 15 2017 Win64"
cmake -G "Visual Studio 15 2017 Win64" -HSource -BBuild -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE%
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%

echo Building 64bit Project in Release mode
cmake --build Build --config Release --target PACKAGE
if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%
