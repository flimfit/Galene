project(flim-ui) 

cmake_minimum_required(VERSION 3.0)

find_package(Qt5Widgets 5.6)
find_package(Qt5PrintSupport 5.6)
find_package(Qt5OpenGL 5.6)
find_package(Qt5Network 5.6)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)

set(CMAKE_AUTOMOC ON)

# QT stuff
#====================================
add_definitions(${QT_DEFINITIONS}) 

qt5_wrap_ui(UI_HEADERS 
			FlimDisplay.ui
			BHRatesWidget.ui
         LifetimeDisplayWidget.ui)

qt5_add_resources(UI_RESOURCES Resources.qrc)

set(SOURCE
	FlimDisplay.cpp
	Main.cpp
	qcustomplot.cpp
   CronologicControlDisplay.cpp
)

set(HEADERS
   BHRatesWidget.h
   FlimDisplay.h
   qcustomplot.h
   Oscilloscope.h
   LifetimeDisplayWidget.h
   FlimWorkspace.h
   CronologicControlDisplay.h
   FifoTcspcControlDisplayFactory.h
   FlimServer.h
)

include_directories(${COMMON_INCLUDE_DIR}
					${fifo_flim_INCLUDE_DIR}
					${InstrumentControl_INCLUDE_DIR}
               ${InstrumentControlUI_INCLUDE_DIR}
               ${QT_USE_FILE}
               ${CMAKE_CURRENT_SOURCE_DIR}
					${CMAKE_CURRENT_BINARY_DIR})

add_executable(flim-ui ${SOURCE} ${HEADERS} 
                                 ${UI_HEADERS} ${UI_RESOURCES} 
                                 $<TARGET_OBJECTS:InstrumentControl> 
                                 $<TARGET_OBJECTS:InstrumentControlUI> 
                                 $<TARGET_OBJECTS:fifo_flim>)

qt5_use_modules(flim-ui Widgets PrintSupport SerialPort Network)

target_link_libraries(flim-ui
   ${OpenCV_LIBS} 
   ${fifo_flim_LIBS} 
   ${InstrumentControl_LIBRARIES})
