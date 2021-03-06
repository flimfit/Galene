cmake_minimum_required (VERSION 3.7)

# For MSVC and FindThreads, respectively.
enable_language(CXX C)

# Prefer C++17 support
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED FALSE)
endif()

include(CheckLanguage)

if(DEFINED ENV{OME_FILES_ROOT})
   set(OME_FILES_ROOT "$ENV{OME_FILES_ROOT}" CACHE PATH "OME root file directory")
endif()

# Download bioformats exporter tool
set(BFTOOLS_JAR "${CMAKE_SOURCE_DIR}/../bftools/bioformats-exporter.jar")
if(NOT EXISTS ${BFTOOLS_JAR})
   file(DOWNLOAD "https://storage.googleapis.com/flimfit-downloads/bioformats-exporter/bioformats-exporter-1.1.0.jar" ${BFTOOLS_JAR})
endif()


set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${OME_FILES_ROOT})

include ("cmake/GitVersion.cmake") # Get version number from git
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
project (Galene LANGUAGES CXX CUDA)

include("cmake/ConfigureMSVCRuntime.cmake")
configure_msvc_runtime("static")

add_definitions(-DVERSION="${VERSION}")
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
add_definitions(-DBOOST_NO_CXX11_SCOPED_ENUMS) # Support older boost
cmake_policy(SET CMP0071 OLD)

check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
   enable_language(CUDA)
else()
   message(STATUS "No CUDA support")
endif()

if(MSVC)
   set(PLATFORM_FLAGS_ALL "/Oi /fp:fast /MP4")
   set(PLATFORM_FLAGS_RELEASE "/O2 /GS- /DYNAMICBASE")
   set(PLATFORM_FLAGS_RELWITHDEBINFO "/O2")
   set(PLATFORM_FLAGS_DEBUG "/ZI")
   add_definitions(-D_CRT_SECURE_NO_WARNINGS)
   add_definitions(-D_MSVC)
endif(MSVC)

if(UNIX)
   set(PLATFORM_FLAGS_ALL "-msse3 -std=c++11 -fPIC")
   set(PLATFORM_FLAGS_RELEASE "-O3 -ffast-math -ftree-vectorize")
   set(PLATFORM_FLAGS_DEBUG "")
endif(UNIX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PLATFORM_FLAGS_ALL}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${PLATFORM_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${PLATFORM_FLAGS_DEBUG}")


# Set output directory for FlimReaderMex file.
# This will be used for generating the figures,
# Matlab is not required for the main application
set(FrameAlignerMex_OUT_DIR ${CMAKE_SOURCE_DIR}/../Matlab)

add_subdirectory(QSimpleUpdater)
add_subdirectory(QtInstrumentControl/InstrumentControl)
add_subdirectory(QtInstrumentControl/InstrumentControlUI)
add_subdirectory(FlimReader/frame-aligner/FrameAligner)
add_subdirectory(FlimReader/frame-aligner/FrameAlignerMex)
add_subdirectory(FlimReader/FlimReader)
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
set(CPACK_PACKAGE_EXECUTABLES Galene "Galene ${VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENCE.txt)

if (WIN32)
   set(CPACK_GENERATOR NSIS)
   set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui/flim-ui.ico")
   set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui/flim-ui.ico")
   set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
   include(CPack)
endif()

