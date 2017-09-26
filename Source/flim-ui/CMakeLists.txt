project(flim-ui) 
cmake_minimum_required(VERSION 3.7)

find_package(Qt5Widgets)
find_package(Qt5PrintSupport)
find_package(Qt5OpenGL)
find_package(Qt5Network)
find_package(OpenMP REQUIRED)
find_package(dlib REQUIRED)

set(OME_ROOT ../../ome-files-build/stage/)
set(OME_FILES_INCLUDE_DIRS ${OME_ROOT}/include)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

include("cmake/DeployQt.cmake")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(CMAKE_AUTOMOC ON)
add_definitions(${QT_DEFINITIONS}) 

qt5_wrap_ui(UI_HEADERS 
         FlimDisplay.ui
         BHRatesWidget.ui
         LifetimeDisplayWidget.ui
         RealignmentStudio.ui
         RealignmentDisplayWidget.ui
         MetaDataDialog.ui)

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
   CronologicControlDisplay.cpp
   SimTcspcControlDisplay.cpp
   RealignmentDisplayWidget.cpp
   RealignmentResultsWriter.cpp
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
   CronologicControlDisplay.h
   SimTcspcControlDisplay.h
   RealignmentResultsWriter.h
   MetaDataDialog.h
)

include_directories(${fifo_flim_INCLUDE_DIR}
                    ${InstrumentControl_INCLUDE_DIR}
                    ${InstrumentControlUI_INCLUDE_DIR}
                    ${FlimReader_INCLUDE_DIR}
                    ${QSimpleUpdater_INCLUDE_DIR}
                    ${QT_USE_FILE}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${OME_FILES_INCLUDE_DIRS})

if(WIN32)
   set(RESOURCE_FILES icon.rc)
elseif(APPLE)
    set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui.icns")
    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(RESOURCE_FILES ${ICON_FILE})
endif()

add_definitions(${fifo_flim_DEFINITIONS})

add_executable(GaleneTest 
                    ${SOURCE} 
                    ${HEADERS}
                    ${UI_HEADERS} 
                    ${UI_RESOURCES} 
                    $<TARGET_OBJECTS:InstrumentControl> 
                    $<TARGET_OBJECTS:InstrumentControlUI> 
                    $<TARGET_OBJECTS:fifo_flim>
                    $<TARGET_OBJECTS:FlimReader>
                    $<TARGET_OBJECTS:FrameAligner>
                    $<TARGET_OBJECTS:QSimpleUpdater>
                    ${RESOURCE_FILES})


add_executable(Galene MACOSX_BUNDLE 
                    ${SOURCE} 
                    ${HEADERS}
                    ${UI_HEADERS} 
                    ${UI_RESOURCES} 
                    $<TARGET_OBJECTS:InstrumentControl> 
                    $<TARGET_OBJECTS:InstrumentControlUI> 
                    $<TARGET_OBJECTS:fifo_flim>
                    $<TARGET_OBJECTS:FlimReader>
                    $<TARGET_OBJECTS:FrameAligner>
                    $<TARGET_OBJECTS:QSimpleUpdater>
                    ${RESOURCE_FILES})

target_link_libraries(Galene
   ${OpenCV_LIBS}
   ${fifo_flim_LIBS} 
   ${InstrumentControl_LIBRARIES}
   ${FlimReader_LIBRARIES}
   ${dlib_LIBRARIES}
   ${libtiff_LIBRARIES})

target_link_libraries(GaleneTest
   ${OpenCV_LIBS}
   ${fifo_flim_LIBS} 
   ${InstrumentControl_LIBRARIES}
   ${FlimReader_LIBRARIES}
   ${dlib_LIBRARIES}
   ${libtiff_LIBRARIES})

add_custom_command(
        TARGET Galene POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/simulated_intensity.png
                ${CMAKE_CURRENT_BINARY_DIR}/simulated_intensity.png)

set_target_properties(Galene PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
                                         MACOSX_BUNDLE_ICON_FILE flim-ui.icns)

qt5_use_modules(Galene Widgets PrintSupport SerialPort Network)
qt5_use_modules(GaleneTest Widgets PrintSupport SerialPort Network)

if (WIN32)
    windeployqt(Galene INCLUDE_MODULES ${QTLIBS})
elseif(APPLE)
    macdeployqt(Galene INCLUDE_MODULES ${QTLIBS})
endif()

set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

install(TARGETS Galene RUNTIME DESTINATION bin 
                        BUNDLE DESTINATION bundle)

if (WIN32)
    install(DIRECTORY $<TARGET_FILE_DIR:Galene>/ DESTINATION bin)
endif()
