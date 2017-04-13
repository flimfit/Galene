cmake_minimum_required (VERSION 3.7) 

set(CMAKE_CXX_STANDARD 14)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.18.35.tar.gz"
    SHA1 "8f75b22def52aa6918f12e66c0617306c9d3af6f"
    LOCAL
)

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

project (flim-ui) 

find_package(Boost REQUIRED COMPONENTS system filesystem)
if(NOT Boost_FOUND)
    hunter_add_package(Boost COMPONENTS filesystem system)
endif()

find_package(OpenCV)
if(NOT OpenCV_FOUND)
    hunter_add_package(OpenCV)
endif()

find_package(Qt5SerialPort)
if(NOT Qt5SerialPort_FOUND)
    hunter_add_package(Qt COMPONENTS qtbase qtserialport)
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Common)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DSUPPRESS_OPENCV_HIGHGUI")

add_subdirectory(QSimpleUpdater)
add_subdirectory(QtInstrumentControl/InstrumentControl)
add_subdirectory(QtInstrumentControl/InstrumentControlUI)
add_subdirectory(FLIMreader/Source/FLIMreader)
add_subdirectory(fifo-flim)
add_subdirectory(flim-ui)

# Create installer
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})

set(CPACK_PACKAGE_NAME "flim_ui")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FLIM UI Installer")
set(CPACK_PACKAGE_VERSION ${VERSION}) # Version of installer
set(CPACK_PACKAGE_VENDOR "Sean Warren")
set(CPACK_PACKAGE_EXECUTABLES flim_ui "flim-ui ${VERSION}")

if (WIN32)
   set(CPACK_GENERATOR NSIS)
   set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui/flim-ui.ico")
   set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui/flim-ui.ico")
   set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
    include(CPack)
endif()

