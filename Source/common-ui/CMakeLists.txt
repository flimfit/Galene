cmake_minimum_required(VERSION 3.7)

project(common-ui LANGUAGES CXX)

find_package(Qt5 REQUIRED COMPONENTS Widgets PrintSupport OpenGL Network)

set(CMAKE_AUTOMOC ON)
cmake_policy(SET CMP0071 OLD)

qt5_wrap_ui(UI_HEADERS
            LifetimeDisplayWidget.ui
            MetaDataDialog.ui
            RealignmentParametersWidget.ui)

set(SOURCE FlimWorkspace.cpp
           LifetimeDisplayWidget.cpp
           qcustomplot.cpp
           UiUtils.cpp
           FlimDataSource.cpp
           RealignableDataSource.cpp
           RealignmentResultsWriter.cpp)

set(HEADERS FlimWorkspace.h
            LifetimeDisplayWidget.h
            qcustomplot.h
            MetaDataDialog.h
            UiUtils.h
            FlimDataSource.h
            RealignableDataSource.h
            RealignmentResultsWriter.h
            RealignmentParametersWidget.h)

set (LIBS InstrumentControlUI
          QSimpleUpdater
          fifo-flim
          FlimReader
          Qt5::Core
          Qt5::Widgets
          Qt5::PrintSupport
          Qt5::Network)

if(WIN32)
    list(APPEND LIBS Qt5::WinMain
                     Qt5::QWindowsIntegrationPlugin)
elseif (APPLE)
    list(APPEND LIBS Qt5::QCocoaIntegrationPlugin)
endif()

add_library(common-ui ${SOURCE} ${HEADERS} ${UI_HEADERS})
target_link_libraries(common-ui ${LIBS})
target_include_directories(common-ui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
                                     PUBLIC    ${CMAKE_CURRENT_BINARY_DIR})
