cmake_minimum_required(VERSION 3.7)

project(flim-ui LANGUAGES CXX)

find_package(Qt5 REQUIRED COMPONENTS Widgets PrintSupport OpenGL Network SerialPort)

set(CMAKE_AUTOMOC ON)
cmake_policy(SET CMP0071 OLD)

set_target_properties(double-conversion::double-conversion
                      PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

qt5_wrap_ui(UI_HEADERS
         FlimDisplay.ui)

set(SOURCE
   FlimStatusWidget.cpp
   FlimDisplay.cpp
   FlimServer.cpp
   Main.cpp
   CronologicControlDisplay.cpp
   FlimServer.cpp
   SimTcspcControlDisplay.cpp
   LiveFlimDataSource.cpp
)

set(HEADERS
   FlimStatusWidget.h
   FlimDisplay.h
   CronologicControlDisplay.h
   FifoTcspcControlDisplayFactory.h
   FlimServer.h
   FlimServerInterface.h
   SimTcspcControlDisplay.h
   LiveFlimDataSource.h
)

include_directories(${QT_USE_FILE}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})


add_executable(flim-ui ${SOURCE} ${HEADERS} ${UI_HEADERS})
target_link_libraries(flim-ui fifo-flim common-ui Qt5::SerialPort)

add_custom_command(
    TARGET flim-ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/simulated_intensity.png
                ${CMAKE_CURRENT_BINARY_DIR}/simulated_intensity.png)

set_target_properties(flim-ui PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
                                         MACOSX_BUNDLE_ICON_FILE flim-ui.icns)


install(TARGETS flim-ui RUNTIME DESTINATION bin
                        BUNDLE DESTINATION bundle)

if (WIN32)
    install(DIRECTORY $<TARGET_FILE_DIR:flim-ui>/ DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/simulated_intensity.png DESTINATION /)
endif()
