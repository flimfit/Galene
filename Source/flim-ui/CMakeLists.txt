project(flim-ui) 

cmake_minimum_required(VERSION 2.8.10)

find_package(Qt5Widgets)
find_package(Qt5PrintSupport)
find_package(Qt5OpenGL)

find_package(OpenCV)

set(CMAKE_AUTOMOC ON)

# QT stuff
#====================================
add_definitions(${QT_DEFINITIONS}) 

qt5_wrap_ui(UI_HEADERS 
			FlimDisplay.ui
			BHRatesWidget.ui)

qt5_add_resources(UI_RESOURCES Resources.qrc)

set(SOURCE
	FlimDisplay.cpp
	Main.cpp
	qcustomplot.cpp
)

set(HEADERS
   BHRatesWidget.h
   FlimDisplay.h
   qcustomplot.h
   Oscilloscope.h
)

include_directories(
					${COMMON_INCLUDE_DIR}
					${fifo_flim_INCLUDE_DIR}
					${InstrumentControl_INCLUDE_DIR}
               ${InstrumentControlUI_INCLUDE_DIR}
               ${QT_USE_FILE}
					${CMAKE_CURRENT_BINARY_DIR})

add_executable(flim-ui ${SOURCE} ${HEADERS} 
                                      ${UI_HEADERS} ${UI_RESOURCES} 
                                      $<TARGET_OBJECTS:InstrumentControl> 
                                      $<TARGET_OBJECTS:InstrumentControlUI> 
                                      $<TARGET_OBJECTS:fifo_flim>)

qt5_use_modules(flim-ui Widgets PrintSupport SerialPort)

target_link_libraries(flim-ui
   ${OpenCV_LIBS} 
   ${fifo_flim_LIBS} 
   ${InstrumentControl_LIBRARIES})
