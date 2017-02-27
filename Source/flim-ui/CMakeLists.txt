project(flim_ui) 

cmake_minimum_required(VERSION 3.7)

find_package(Qt5Widgets)
find_package(Qt5PrintSupport)
find_package(Qt5OpenGL)
find_package(Qt5Network)
find_package(OpenMP REQUIRED)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

include("cmake/DeployQt.cmake")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(CMAKE_AUTOMOC ON)

if(USE_CRONOLOGIC)
    set(CRONOLOGIC_FILES "CronologicControlDisplay.cpp CronologicControlDisplay.h")
endif()

# QT stuff
#====================================
add_definitions(${QT_DEFINITIONS}) 

qt5_wrap_ui(UI_HEADERS 
         FlimDisplay.ui
         BHRatesWidget.ui
         LifetimeDisplayWidget.ui
         RealignmentStudio.ui
         RealignmentDisplayWidget.ui)

qt5_add_resources(UI_RESOURCES Resources.qrc)

set(SOURCE
   FlimDisplay.cpp
   FlimServer.cpp
   Main.cpp
   qcustomplot.cpp
   
   RealignmentStudio.cpp
   LifetimeDisplayWidget.cpp
   FlimReaderDataSource.cpp
   FlimWorkspace.cpp
)

set(HEADERS
   BHRatesWidget.h
   FlimDisplay.h
   qcustomplot.h
   Oscilloscope.h
   LifetimeDisplayWidget.h
   FlimWorkspace.h
   FifoTcspcControlDisplayFactory.h
   FlimServer.h
   FlimReaderDataSource.h
   RealignmentStudio.h
   RealignmentDisplayWidget.h
   RealignmentImageSource.h
   RealignmentStudioBatchProcessor.h
)

include_directories(${fifo_flim_INCLUDE_DIR}
                    ${InstrumentControl_INCLUDE_DIR}
                    ${InstrumentControlUI_INCLUDE_DIR}
                    ${FLIMreader_INCLUDE_DIR}
                    ${QSimpleUpdater_INCLUDE_DIR}
                    ${QT_USE_FILE}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
   set(RESOURCE_FILES icon.rc)
elseif(APPLE)
    set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui.icns")
    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(RESOURCE_FILES ${ICON_FILE})
endif()

add_executable(flim_ui MACOSX_BUNDLE 
                    ${SOURCE} ${HEADERS} ${CRONOLOGIC_FILES}
                    ${UI_HEADERS} ${UI_RESOURCES} 
                    $<TARGET_OBJECTS:InstrumentControl> 
                    $<TARGET_OBJECTS:InstrumentControlUI> 
                    $<TARGET_OBJECTS:fifo_flim>
                    $<TARGET_OBJECTS:FLIMreader>
                    $<TARGET_OBJECTS:QSimpleUpdater>
                    ${RESOURCE_FILES})

target_link_libraries(flim_ui
   ${OpenCV_LIBS}
   ${fifo_flim_LIBS} 
   ${InstrumentControl_LIBRARIES}
   ${FLIMreader_LIBRARIES})

set_target_properties(flim_ui PROPERTIES MACOSX_BUNDLE_ICON_FILE flim-ui.icns)

qt5_use_modules(flim_ui Widgets PrintSupport SerialPort Network)

if (WIN32)
    windeployqt(flim_ui INCLUDE_MODULES ${QTLIBS})
elseif(APPLE)
    macdeployqt(flim_ui INCLUDE_MODULES ${QTLIBS})
endif()


set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

install(TARGETS flim_ui RUNTIME DESTINATION bin 
                        BUNDLE DESTINATION bundle)

if (WIN32)
    install(DIRECTORY $<TARGET_FILE_DIR:flim_ui>/ DESTINATION bin)
endif()
