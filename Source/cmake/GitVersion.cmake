
# Get current version from git
find_package(Git)
set(VERSION "0.0.0")
if(GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_SOURCE_DIR} describe
            OUTPUT_VARIABLE VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
message(STATUS "VERSION: ${VERSION}")
string(REGEX MATCH "([0-9]+)\.([0-9]+)\.([0-9]+)" MATCHED ${VERSION})

set(VERSION_MAJOR ${CMAKE_MATCH_1})
set(VERSION_MINOR ${CMAKE_MATCH_2})
set(VERSION_PATCH ${CMAKE_MATCH_3})
