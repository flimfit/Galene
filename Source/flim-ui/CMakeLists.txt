project(flim-ui) 

cmake_minimum_required(VERSION 3.3)

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
         LifetimeDisplayWidget.ui
         RealignmentStudio.ui)

qt5_add_resources(UI_RESOURCES Resources.qrc)

set(SOURCE
   FlimDisplay.cpp
   FlimServer.cpp
   Main.cpp
   qcustomplot.cpp
   CronologicControlDisplay.cpp
   RealignmentStudio.cpp
   LifetimeDisplayWidget.cpp
   FlimReaderDataSource.cpp
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
   FlimReaderDataSource.h
   RealignmentStudio.h
)

include_directories(${fifo_flim_INCLUDE_DIR}
                    ${InstrumentControl_INCLUDE_DIR}
                    ${InstrumentControlUI_INCLUDE_DIR}
                    ${FLIMreader_INCLUDE_DIR}
                    ${QT_USE_FILE}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

add_executable(flim-ui ${SOURCE} ${HEADERS} 
                                 ${UI_HEADERS} ${UI_RESOURCES} 
                                 $<TARGET_OBJECTS:InstrumentControl> 
                                 $<TARGET_OBJECTS:InstrumentControlUI> 
                                 $<TARGET_OBJECTS:fifo_flim>
                                 $<TARGET_OBJECTS:FLIMreader>)

target_link_libraries(flim-ui
   ${OpenCV_LIBS} 
   ${fifo_flim_LIBS} 
   ${InstrumentControl_LIBRARIES}
   ${FLIMreader_LIBRARIES})

qt5_use_modules(flim-ui Widgets PrintSupport SerialPort Network)
