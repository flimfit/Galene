cmake_minimum_required(VERSION 3.7)

project(flim-ui LANGUAGES CXX)

find_package(Qt5 REQUIRED COMPONENTS Widgets PrintSupport OpenGL Network SerialPort)
find_package(Boost REQUIRED COMPONENTS log iostreams thread)
find_package(OMEFiles REQUIRED)
find_package(bioimage REQUIRED)
find_package(libics REQUIRED)

set_target_properties(OME::Files OME::XML OME::Common 
                      OME::Compat OME::XalanUtil OME::XercesUtil 
                      bioimage jzon pole pugixml exiv2lib xmp double-conversion::double-conversion
                      PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

include_directories(${OME_FILES_ROOT}/include)

find_package(HDF5 CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
cmake_policy(SET CMP0071 OLD)

qt5_wrap_ui(UI_HEADERS
         RealignmentStudio.ui
         RealignmentDisplayWidget.ui
)

set(SOURCE
   Main.cpp
   RealignmentStudio.cpp
   FlimReaderDataSource.cpp
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
   SpectralCorrectionListModel.cpp
   RealignmentStudioBatchProcessor.cpp
   Galene.manifest
)

set(HEADERS
   FlimReaderDataSource.h
   IntensityDataSource.h
   RealignmentStudio.h
   RealignmentDisplayWidget.h
   RealignmentStudioBatchProcessor.h
   RealignmentResultsWriter.h
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
elseif(APPLE)
    set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Galene.icns")
    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(RESOURCE_FILES ${ICON_FILE})
endif()

set(HDF5_LIBRARIES hdf5::hdf5-static) # for vcpkg

set (Galene_OBJECTS ${SOURCE}
                    ${HEADERS}
                    ${UI_HEADERS}
                    ${RESOURCE_FILES})

set (Galene_LIBS    ${HDF5_LIBRARIES}
                    bioimage
                    fifo-flim
                    common-ui
                    FlimReader
                    InstrumentControlUI
                    QSimpleUpdater
                    OME::Files
                    libics
                    Qt5::Core
                    Qt5::Widgets
                    Qt5::PrintSupport
                    Qt5::SerialPort
                    Qt5::Network)

if(WIN32)
    list(APPEND Galene_LIBS Qt5::WinMain
                            Qt5::QWindowsIntegrationPlugin)
elseif (APPLE)
    list(APPEND Galene_LIBS Qt5::QCocoaIntegrationPlugin)
endif()

if (APPLE) # easier debugging through vs code
    add_executable(GaleneTest ${Galene_OBJECTS})
    target_link_libraries(GaleneTest ${Galene_LIBS})
    set_property(TARGET GaleneTest PROPERTY CUDA_SEPARABLE_COMPILATION ON)
    target_link_options(GaleneTest PUBLIC -fvisibility=default)
endif()

add_executable(Galene MACOSX_BUNDLE ${Galene_OBJECTS})
target_link_libraries(Galene ${Galene_LIBS})


if (APPLE)
    target_link_options(Galene PUBLIC -fvisibility=default)
endif()

add_custom_command(
    TARGET Galene POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${OME_FILES_ROOT}/share/xml
                $<TARGET_FILE_DIR:Galene>/share/xml)

add_custom_command(
    TARGET Galene POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${OME_FILES_ROOT}/share/xsl
                $<TARGET_FILE_DIR:Galene>/share/xsl)

add_custom_command(
    TARGET Galene POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
                ${BFTOOLS_JAR} 
                $<TARGET_FILE_DIR:Galene>/bioformats-exporter.jar)


set_target_properties(Galene PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
                                        MACOSX_BUNDLE_ICON_FILE Galene.icns)


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
endif()
