cmake_minimum_required (VERSION 3.5) 

set(CMAKE_CXX_STANDARD 14)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.14.29.tar.gz"
    SHA1 "9c29c30fff99b7c1479b2e9d4926dcc3f8d364e0"
)

include("cmake/DeployQt.cmake")

find_package(Git)
if(GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_SOURCE_DIR} describe
            OUTPUT_VARIABLE VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "VERSION: ${VERSION}")
endif()

project (flim-ui) 
hunter_add_package(Boost COMPONENTS filesystem system)
hunter_add_package(OpenCV)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Common)

set(USE_CRONOLOGIC ON)

add_subdirectory(QtInstrumentControl/InstrumentControl)
add_subdirectory(QtInstrumentControl/InstrumentControlUI)
add_subdirectory(FLIMreader/Source/FLIMreader)
add_subdirectory(fifo-flim)
add_subdirectory(flim-ui)

include(CPack)