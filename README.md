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
- Qt 5.7 or greater, download from https://www.qt.io/download-open-source/
- vcpkg, for installing required dependencies, install from https://github.com/Microsoft/vcpkg
- choco, for installing required dependencies, install from https://chocolatey.org/

Install the required dependencies using
```
vcpkg install opencv:x64-windows fftw3:x64-windows boost:x64-windows xalan-c:x64-windows exiv2:x64-windows libraw:x64-windows eigen3:x64-windows proj4:x64-windows hdf5:x64-windows
choco install cmake python2 pip nsis ninja -y
pip install genshi 
```

- Add an environment variable to point CMake to Qt, e.g. 
   `CMAKE_PREFIX_PATH = C:\Qt\5.9\msvc2017_64`
- Add an environment variable `VCPKG_ROOT` to point to the vcpkg install  

Build steps
- Clone the Galene repository 
- Run script `build-libs.bat` to build `bioimage` and `ome-files`
    - run `build-libs.bat --build-debug` to also build debug and relwithdebinfo libraries
- Run script `build-flim-ui.bat`
- A Visual Studio solution will be created in the `Build` subfolder

Building on Mac
-------------------
Requirements
- MacOS 10.12 or greater
- XCode 8 or greater
- Homebrew, install from https://brew.sh

Install the required dependencies using
```
brew install qt cmake opencv boost@1.60 xerces-c xalan-c exiv2 libraw` 
```
Build steps
- Install LLVM, Qt and cmake using [homebrew](http://brew.sh)
    - `brew install qt cmake opencv boost@1.60 xerces-c xalan-c exiv2 libraw` 
- Clone the Galene repository 
- Run script `build-flim-ui.sh`

Acknowledgements
-------------------
Galene dynamically links to Qt 5.9, [licenced under the LGPLv3](http://doc.qt.io/qt-5/lgpl.html). 
Qt is available from [Qt.io](http://qt.io), or you can download the source code used in Galene [here](http://downloads.flimfit.org/qt/qt59.tar.xz).
