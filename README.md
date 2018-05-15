Galene
======

Galene is a tool to correct for motion in fluorescence lifetime imaging (FLIM) data. For more information please see [citation]. 

Precompiled applications are available from http://www.flimfit.org/galene.

For documentation please see http://galene.readthedocs.io.

Instructions to compile from source are provided below, including retrieving the required dependencies.  

Building on Windows
-------------------
Requirements
- Windows 7 or greater
- Visual Studio 2015 or greater
- Qt 5.7 or greater, install from https://www.qt.io/download-open-source/
- vcpkg, for installing required dependencies, install from https://github.com/Microsoft/vcpkg
- choco, for installing required dependencies, install from https://chocolatey.org/
- Optionally, CUDA to support GPU processing, install from https://developer.nvidia.com/cuda-downloads

Install the required dependencies using
```
vcpkg install opencv:x64-windows fftw3:x64-windows boost:x64-windows xalan-c:x64-windows exiv2:x64-windows libraw:x64-windows eigen3:x64-windows proj4:x64-windows hdf5:x64-windows
choco install cmake python2 pip nsis ninja -y
pip install genshi 
```

- Add an environment variable to point CMake to Qt, e.g. 
   `CMAKE_PREFIX_PATH = C:\Qt\5.9\msvc2017_64`
- Add an environment variable `VCPKG_ROOT` to point to the vcpkg folder  

Build steps
- Clone the Galene repository from `https://github.com/flimfit/galene`
- Run script `build-libs.bat` to build `bioimage` and `ome-files`
    - run `build-libs.bat --build-debug` to also build debug and relwithdebinfo libraries
- Run script `build-flim-ui.bat`
- A Visual Studio solution will be created in the `Build` subfolder

Building on Mac
-------------------
Requirements
- MacOS 10.11 or greater
- XCode 8.2 or greater
- Homebrew, install from https://brew.sh
- Optionally, CUDA to support GPU processing, install from https://developer.nvidia.com/cuda-downloads
    - Download CUDA 8 for macOS 10.11 or CUDA 9.1 for macOS 10.12. 
    - macOS 10.13 is not currently supported for building CUDA applications 

Install the required dependencies using
```
brew install qt cmake opencv boost xerces-c xalan-c exiv2 libraw eigen libpng libtiff proj hdf5 fftw dlib zlib python ninja npm
pip install genshi 
sudo npm install -g appdmg # to build the disk image
```

If you have some of the packages installed already, `brew upgrade` to retrieve the latest packages


Build steps
- Clone the Galene repository from `https://github.com/flimfit/galene`
- Run script `build-flim-ui.sh`

Acknowledgements
-------------------
Galene uses the following open source software. 

| Package          |                                              | Licence  |
| -----------------|----------------------------------------------| ---------|
| Qt               | http://www.qt.io                             | LGPL v3  |
| ome-files-cpp	   | https://www.openmicroscopy.org/ome-files/    | MIT      |
| bioimageconvert  | https://bitbucket.org/dimin/bioimageconvert/ | MIT      |
| dlib             | http://dlib.net/                             | Boost    |
| OpenCV           | http://opencv.org/                           | BSD      |
| boost            | http://www.boost.org/                        | Boost    |
| fftw3            | http://www.fftw.org/                         | GPL v2   |
| libhdf5          | https://www.hdfgroup.org/                    | BSD      |
