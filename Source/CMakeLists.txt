cmake_minimum_required (VERSION 3.7) 
set(CMAKE_CXX_STANDARD 14)

# Use hunter to acquire dependencies where we can 
include("cmake/HunterGate.cmake")
HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.18.43.tar.gz"
    SHA1 "d2c8c42cd07f7cefe18fd9a9b9c13114b1a15a27"
    LOCAL
)

# Get current version from git
find_package(Git)
if(GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_SOURCE_DIR} describe
            OUTPUT_VARIABLE VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "VERSION: ${VERSION}")
	string(REGEX MATCH "([0-9]+)\.([0-9]+)\.([0-9]+)" MATCHED ${VERSION})
	
	set(VERSION_MAJOR ${CMAKE_MATCH_1})
	set(VERSION_MINOR ${CMAKE_MATCH_2})
	set(VERSION_PATCH ${CMAKE_MATCH_3})
	add_definitions(-DVERSION="${VERSION}")
endif()

project (Galene) 

find_package(Boost COMPONENTS system filesystem)
if(NOT Boost_FOUND)
    hunter_add_package(Boost COMPONENTS filesystem system)
endif()

hunter_add_package(OpenCV)

find_package(dlib)
if(NOT dlib_FOUND)
   hunter_add_package(dlib)
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

# Work around bug with OpenCV highgui components when compiled with ReleaseWithDebugInfo
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DSUPPRESS_OPENCV_HIGHGUI")

# Set output directory for FlimReaderMex file
# This will be used for generating the figures
set(FlimReaderMEX_OUT_DIR ${CMAKE_SOURCE_DIR}/Matlab/../)

add_subdirectory(QSimpleUpdater)
add_subdirectory(QtInstrumentControl/InstrumentControl)
add_subdirectory(QtInstrumentControl/InstrumentControlUI)
add_subdirectory(FlimReader/Source/FlimReader)
add_subdirectory(FlimReader/Source/FlimReaderMex)
add_subdirectory(FlimReader/Source/FlimReaderTest)
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

