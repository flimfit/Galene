cmake_minimum_required (VERSION 3.7) 
set(CMAKE_CXX_STANDARD 14)

if(DEFINED ENV{OME_FILES_ROOT})
    set(OME_FILES_ROOT "$ENV{OME_FILES_ROOT}")
else()
    set(OME_FILES_ROOT "${CMAKE_SOURCE_DIR}/../ome-files-install")
endif()

set(CMAKE_PREFIX_PATH "${OME_FILES_ROOT}")

#include ("cmake/TargetAppleSdk.cmake")
#target_apple_sdk(10.10)

include ("cmake/GitVersion.cmake") # Get version number from git

project (Galene) 

add_definitions(-DVERSION=${VERSION})
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

find_package(Boost REQUIRED)
find_package(OpenCV REQUIRED)

find_package(MATLAB)
if(MATLAB_FOUND)
    # Set output directory for FlimReaderMex file.
    # This will be used for generating the figures,
    # Matlab is not required for the main application
    set(FlimReaderMEX_OUT_DIR ${CMAKE_SOURCE_DIR}/../Matlab)
endif()

set(DLIB_USE_BLAS ON)
set(DDLIB_USE_LAPACK ON)
set(DLIB_USE_CUDA OFF)
set(DLIB_JPEG_SUPPORT OFF)
set(DLIB_LINK_WITH_SQLITE3 OFF)
set(DLIB_PNG_SUPPORT OFF)
set(DLIB_GIF_SUPPORT OFF)
set(DLIB_NO_GUI_SUPPORT ON)
set(DLIB_IN_PROJECT_BUILD ON)

add_subdirectory(dlib)
add_subdirectory(QSimpleUpdater)
add_subdirectory(QtInstrumentControl/InstrumentControl)
add_subdirectory(QtInstrumentControl/InstrumentControlUI)
add_subdirectory(FlimReader/frame-aligner/FrameAligner)
add_subdirectory(FlimReader/FlimReader)
#add_subdirectory(FlimReader/FlimReaderTest)
add_subdirectory(fifo-flim)
add_subdirectory(flim-ui)

# Create installer
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})

set(CPACK_PACKAGE_NAME "Galene")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Galene Installer")
set(CPACK_PACKAGE_VERSION ${VERSION}) # Version of installer
set(CPACK_PACKAGE_VENDOR "Sean Warren")
set(CPACK_PACKAGE_EXECUTABLES Galene "Galene")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENCE.txt)

if (WIN32)
   set(CPACK_GENERATOR NSIS)
   set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui/flim-ui.ico")
   set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui/flim-ui.ico")
   set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
    include(CPack)
endif()

