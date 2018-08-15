cmake_minimum_required(VERSION 3.7)
project(flim-ui LANGUAGES CXX) 

find_package(Qt5 REQUIRED COMPONENTS Widgets PrintSupport OpenGL Network SerialPort)
find_package(Boost REQUIRED COMPONENTS log iostreams)
find_package(OMEXML REQUIRED)
find_package(OMEFiles REQUIRED)
find_package(bioimage REQUIRED)
find_package(libics REQUIRED)

find_package(CUDA QUIET)
include_directories(${OME_FILES_ROOT}/include)

find_package(hdf5 REQUIRED)

include("cmake/DeployQt.cmake")

set(CMAKE_AUTOMOC ON)
cmake_policy(SET CMP0071 OLD)

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
   IntensityReader.cpp
   IntensityDataSource.cpp
   RealignableDataSource.cpp
   BioImageIntensityReader.cpp
   OmeIntensityReader.cpp
   OmeIntensityWriter.cpp
   ImarisIntensityReader.cpp
   ImarisIntensityWriter.cpp
   IcsIntensityReader.cpp
   BioformatsExporter.cpp
   Galene.manifest
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
   IntensityDataSource.h
   RealignmentStudio.h
   RealignmentDisplayWidget.h
   RealignmentStudioBatchProcessor.h
   CronologicControlDisplay.h
   SimTcspcControlDisplay.h
   RealignmentResultsWriter.h
   MetaDataDialog.h
   RealignableDataSource.h
   IntensityReader.h
   IntensityDataSource.h
   BioImageIntensityReader.h
   OmeIntensityReader.h
   IntensityWriter.h
   OmeIntensityWriter.h
   ImarisIntensityReader.h
   ImarisIntensityWriter.h
   BioformatsExporter.h
   IcsIntensityReader.h
)

include_directories(${QT_USE_FILE}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${OME_FILES_INCLUDE_DIRS}
                    ${HDF5_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS})

if(WIN32)
   set(RESOURCE_FILES icon.rc)
   set(HDF5_LIBRARIES hdf5::hdf5-shared) # for vcpkg
elseif(APPLE)
    set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/flim-ui.icns")
    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(RESOURCE_FILES ${ICON_FILE})
endif()

add_definitions(${fifo_flim_DEFINITIONS})

set (Galene_OBJECTS ${SOURCE} 
                    ${HEADERS}
                    ${UI_HEADERS} 
                    ${UI_RESOURCES} 
                    ${RESOURCE_FILES})

set (Galene_LIBS    ${CUDA_LIBRARIES}
                    ${LAPACK_LIBRARIES}
                    ${BLAS_LIBRARIES}
                    ${HDF5_LIBRARIES}
                    bioimage
                    fifo_flim
                    FlimReader
                    InstrumentControlUI
                    QSimpleUpdater
                    OME::Files
                    libics
                    Qt5::Widgets 
                    Qt5::PrintSupport 
                    Qt5::SerialPort 
                    Qt5::Network)

if (APPLE) # easier debugging through vs code
    add_executable(GaleneTest ${Galene_OBJECTS})
    target_link_libraries(GaleneTest ${Galene_LIBS})
    set_property(TARGET GaleneTest PROPERTY CUDA_SEPARABLE_COMPILATION ON)
endif()

add_executable(Galene MACOSX_BUNDLE ${Galene_OBJECTS})
target_link_libraries(Galene ${Galene_LIBS})

add_custom_command(
    TARGET Galene POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/simulated_intensity.png
                ${CMAKE_CURRENT_BINARY_DIR}/simulated_intensity.png)

add_custom_command(
    TARGET Galene POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${OME_FILES_ROOT}/share/
                $<TARGET_FILE_DIR:Galene>/share/)

               
add_custom_command(
    TARGET Galene POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${BFTOOLS_DIR}/bioformats-exporter.jar $<TARGET_FILE_DIR:Galene>/bioformats-exporter.jar)


set_target_properties(Galene PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
                                        MACOSX_BUNDLE_ICON_FILE flim-ui.icns)


deployqt(Galene INCLUDE_MODULES ${QTLIBS})

install(TARGETS Galene RUNTIME DESTINATION bin 
                        BUNDLE DESTINATION bundle)                           

if (WIN32)

    if (EXISTS "$ENV{MKLROOT}")
        # install MKL redist's if we used it
        get_filename_component(INTEL_REDIST "$ENV{MKLROOT}/../redist/intel64_win/compiler/" REALPATH)
        install(DIRECTORY "${INTEL_REDIST}/" DESTINATION bin FILES_MATCHING PATTERN "*.dll")
    endif()

    install(DIRECTORY $<TARGET_FILE_DIR:Galene>/ DESTINATION bin)
    install(DIRECTORY $<TARGET_FILE_DIR:Galene>/share DESTINATION /)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/simulated_intensity.png DESTINATION /)
endif()
