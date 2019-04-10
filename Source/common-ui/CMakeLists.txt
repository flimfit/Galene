cmake_minimum_required(VERSION 3.7)

project(common-ui LANGUAGES CXX)

find_package(Qt5 REQUIRED COMPONENTS Widgets PrintSupport OpenGL Network SerialPort)

set(CMAKE_AUTOMOC ON)
cmake_policy(SET CMP0071 OLD)

qt5_wrap_ui(UI_HEADERS
         LifetimeDisplayWidget.ui)

set(SOURCE
   FlimWorkspace.cpp
   LifetimeDisplayWidget.cpp
)

set(HEADERS
    ControlLayout.h
    FlimWorkspace.h
    LifetimeDisplayWidget.h
)

include_directories(${QT_USE_FILE}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set (OBJECTS )

set (LIBS InstrumentControlUI
          QSimpleUpdater
          Qt5::Core
          Qt5::Widgets
          Qt5::PrintSupport
          Qt5::SerialPort
          Qt5::Network)

if(WIN32)
    list(APPEND LIBS Qt5::WinMain
                     Qt5::QWindowsIntegrationPlugin)
elseif (APPLE)
    list(APPEND LIBS Qt5::QCocoaIntegrationPlugin)
endif()

add_library(common-ui ${SOURCE} ${HEADERS} ${UI_HEADERS})
target_link_libraries(common-ui ${LIBS})

