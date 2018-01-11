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
- MacOS 10.12 or greater
- CMake 3.7 or greater (install via brew)
- Qt 5.7 or greater (install via brew)

Build steps
- Install LLVM, Qt and cmake using [homebrew](http://brew.sh)
    - `brew install llvm qt5 cmake boost@1.60 exiv2` 
- Clone the Galene repository 
- Run script `build-flim-ui.sh`

Acknowledgements
-------------------
Galene dynamically links to Qt 5.9, [licenced under the LGPLv3](http://doc.qt.io/qt-5/lgpl.html). 
Qt is available from [Qt.io](http://qt.io), or you can download the source code used in Galene [here](http://downloads.flimfit.org/qt/qt59.tar.xz).
