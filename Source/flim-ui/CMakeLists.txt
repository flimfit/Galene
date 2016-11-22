project(flim_ui) 

cmake_minimum_required(VERSION 3.3)

find_package(Qt5Widgets 5.7)
find_package(Qt5PrintSupport 5.7)
find_package(Qt5OpenGL 5.7)
find_package(Qt5Network 5.7)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)

set(CMAKE_AUTOMOC ON)

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
   CronologicControlDisplay.cpp
   RealignmentStudio.cpp
   LifetimeDisplayWidget.cpp
   FlimReaderDataSource.cpp
   FlimWorkspace.cpp
   FlimDataSource.cpp
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
   RealignmentDisplayWidget.h
   RealignmentImageSource.h
   RealignmentStudioBatchProcessor.h
)

include_directories(${fifo_flim_INCLUDE_DIR}
                    ${InstrumentControl_INCLUDE_DIR}
                    ${InstrumentControlUI_INCLUDE_DIR}
                    ${FLIMreader_INCLUDE_DIR}
                    ${QT_USE_FILE}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
   set(WIN_RESOURCE_FILE icon.rc)
endif()

add_executable(flim_ui ${SOURCE} ${HEADERS} 
                                 ${UI_HEADERS} ${UI_RESOURCES} 
                                 $<TARGET_OBJECTS:InstrumentControl> 
                                 $<TARGET_OBJECTS:InstrumentControlUI> 
                                 $<TARGET_OBJECTS:fifo_flim>
                                 $<TARGET_OBJECTS:FLIMreader> ${WIN_RESOURCE_FILE})

target_link_libraries(flim_ui
   ${OpenCV_LIBS} 
   ${fifo_flim_LIBS} 
   ${InstrumentControl_LIBRARIES}
   ${FLIMreader_LIBRARIES})

qt5_use_modules(flim_ui Widgets PrintSupport SerialPort Network Charts)

DeployQt(TARGET flim_ui INCLUDE_MODULES ${QTLIBS})

set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

install(TARGETS flim_ui RUNTIME DESTINATION bin)
install(DIRECTORY $<TARGET_FILE_DIR:flim_ui>/ DESTINATION bin)


# Create installer

set(CPACK_PACKAGE_NAME "flim_ui" PARENT_SCOPE)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FLIM UI Installer" PARENT_SCOPE)
set(CPACK_PACKAGE_VERSION ${VERSION} PARENT_SCOPE) # Version of installer
set(CPACK_PACKAGE_VENDOR "Sean Warren" PARENT_SCOPE)
set(CPACK_PACKAGE_EXECUTABLES flim_ui "flim-ui ${VERSION}" PARENT_SCOPE)

if (WIN32)
   set(CPACK_GENERATOR NSIS)
   set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui.ico" PARENT_SCOPE)
   set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui.ico" PARENT_SCOPE)
   set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON" PARENT_SCOPE)
elseif (APPLE)
   set(CPACK_GENERATOR BUNDLE)
endif()