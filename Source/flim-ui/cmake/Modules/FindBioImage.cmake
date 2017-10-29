# Search for bioimage headers and libraries
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# search in CMAKE_PREFIX_PATH:
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)

if(BIOIMAGE_FIND_REQUIRED)
    find_package(PkgConfig REQUIRED)
  pkg_search_module(PC_BIOIMAGE bioimage REQUIRED)
else()
    find_package(PkgConfig)
    pkg_search_module(PC_BIOIMAGE bioimage)
endif()


if(PC_BIOIMAGE_FOUND)
    #message("PC_BIOIMAGE_LIBRARIES:      ${PC_BIOIMAGE_LIBRARIES}")
    #message("PC_BIOIMAGE_LIBRARY_DIRS:   ${PC_BIOIMAGE_LIBRARY_DIRS}")
    #message("PC_BIOIMAGE_LDFLAGS:        ${PC_BIOIMAGE_LDFLAGS}")
    #message("PC_BIOIMAGE_LDFLAGS_OTHER:  ${PC_BIOIMAGE_LDFLAGS_OTHER}")
    #message("PC_BIOIMAGE_INCLUDE_DIRS:   ${PC_BIOIMAGE_INCLUDE_DIRS}")
    #message("PC_BIOIMAGE_CFLAGS:         ${PC_BIOIMAGE_CFLAGS}")
    #message("PC_BIOIMAGE_CFLAGS_OTHER:   ${PC_BIOIMAGE_CFLAGS_OTHER}")
    #message("PC_BIOIMAGE:                ${PC_BIOIMAGE}")
    #message("PC_BIOIMAGE_VERSION:        ${PC_BIOIMAGE_VERSION}")
    #message("PC_BIOIMAGE_PREFIX:         ${PC_BIOIMAGE_PREFIX}")
    #message("PC_BIOIMAGE_INCLUDEDIR:     ${PC_BIOIMAGE_INCLUDEDIR}")
    #message("PC_BIOIMAGE_LIBDIR:         ${PC_BIOIMAGE_LIBDIR}")

    set(BIOIMAGE_FOUND TRUE)
    set(BIOIMAGE_INCLUDE_DIRS
        ${PC_BIOIMAGE_INCLUDE_DIRS})
    set(BIOIMAGE_LIBRARIES
        ${PC_BIOIMAGE_LDFLAGS})
endif()

mark_as_advanced(PC_BIOIMAGE PC_BIOIMAGE_FOUND BIOIMAGE_FOUND PC_BIOIMAGE_INCLUDE_DIR PC_BIOIMAGE_LIBRARIES)


#find_path(BIOIMAGE_INCLUDE_DIR bim_format_manager.h HINTS
#          ${PC_BIOIMAGE_INCLUDEDIR} ${PC_BIOIMAGE_INCLUDE_DIRS}
#          /usr/local/include/bioimage /usr/include/bioimage)
#
#find_library(BIOIMAGE_LIBRARY NAMES
#             libbioimage${CMAKE_SHARED_LIBRARY_SUFFIX}
#             libbioimage${CMAKE_STATIC_LIBRARY_SUFFIX}
#             HINTS ${PC_BIOIMAGE_LIBDIR} ${PC_BIOIMAGE_LIBRARY_DIRS})
#
#if(BIOIMAGE_INCLUDE_DIR AND BIOIMAGE_LIBRARY)
#    set(BIOIMAGE_FOUND TRUE)
#    set(BIOIMAGE_INCLUDE_DIRS
#        ${BIOIMAGE_INCLUDE_DIR})
#    set(BIOIMAGE_LIBRARIES
#        ${BIOIMAGE_LIBRARY})
#
#    if(NOT BIOIMAGE_FIND_QUIETLY)
#        message(STATUS "Found BIOIMAGE: ${BIOIMAGE_LIBRARIES}")
#    endif()
#else()
#    set(BIOIMAGE_FOUND FALSE)
#    if(BIOIMAGE_FIND_REQUIRED)
#        message(FATAL_ERROR "Could not find BIOIMAGE (which is required)")
#    endif()
#endif()