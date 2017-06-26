Galene
======

Galene is a tool to correct for motion in fluorescence lifetime imaging (FLIM) data. For more information please see [citation]. 

Precompiled applications are available from http://www.flimfit.org/galene.

Instructions to compile from source are provided below 

Building on Windows
-------------------
Requirements
- Windows 7 or greater
- Visual Studio 2015 or greater
- CMake 3.7 or greater
- Qt 5.7 or greater, download from [Qt website](https://www.qt.io/download-open-source/)

Build steps
- Add an environment variable to point CMake to Qt, e.g. 
   `CMAKE_PREFIX_PATH = C:\Qt\5.9\msvc2017_64`
- Clone the Galene repository 
- Run script `build-flim-ui.bat`
- Open the Visual Studio solution in the `Build` subfolder
- Build this solution in Visual Studio 

Building on Mac
-------------------
Requirements
- MacOS 10.10 or greater
- LLVM 4.0 or greater (install via brew, apple supplied version does not support OpenMP)
- CMake 3.7 or greater (install via brew)
- Qt 5.7 or greater (install via brew)

Build steps
- Install LLVM, Qt and cmake using [homebrew](http://brew.sh)
    - `brew install llvm qt5 cmake` 
- Clone the Galene repository 
- Run script `build-flim-ui.sh`
