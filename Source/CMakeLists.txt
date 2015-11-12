cmake_minimum_required (VERSION 2.8.10) 
project (flim-ui) 

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Common)

add_subdirectory(QtInstrumentControl/InstrumentControl)
add_subdirectory(QtInstrumentControl/InstrumentControlUI)
add_subdirectory(fifo-flim)
add_subdirectory(flim-ui)
