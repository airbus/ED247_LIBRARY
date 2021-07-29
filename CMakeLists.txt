###############################################################################
# The MIT Licence                                                             #
#                                                                             #
# Copyright (c) 2020 Airbus Operations S.A.S                                  #
#                                                                             #
# Permission is hereby granted, free of charge, to any person obtaining a     #
# copy of this software and associated documentation files (the "Software"),  #
# to deal in the Software without restriction, including without limitation   #
# the rights to use, copy, modify, merge, publish, distribute, sublicense,    #
# and/or sell copies of the Software, and to permit persons to whom the       #
# Software is furnished to do so, subject to the following conditions:        #
#                                                                             #
# The above copyright notice and this permission notice shall be included     #
# in all copies or substantial portions of the Software.                      #
#                                                                             #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE #
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     #
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         #
# DEALINGS IN THE SOFTWARE.                                                   #
###############################################################################

cmake_minimum_required(VERSION 3.13)

# Set CMAKE_BUILD_TYPE to Release by default.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()
# SET(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "" FORCE)

project(ED247_LIBRARY)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION ED247_LIBRARY_VERSION)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 11)

# Print some useful information to screen
STRING(TIMESTAMP CMAKE_CONFIGURE_DATE UTC)
MESSAGE("########################################################")
MESSAGE("# Project      ${PROJECT_NAME}")
MESSAGE("# Version      ${ED247_LIBRARY_VERSION}")
MESSAGE("# Build Type   ${CMAKE_BUILD_TYPE}")
MESSAGE("# System Name  ${CMAKE_SYSTEM_NAME}")
MESSAGE("# Processor    ${CMAKE_SYSTEM_PROCESSOR}")
MESSAGE("# Directory    ${CMAKE_SOURCE_DIR}")
MESSAGE("# Date         ${CMAKE_CONFIGURE_DATE}")
MESSAGE("########################################################")

# Options
option(BUILD_SAMPLES        "Build samples"                                 OFF)
option(BUILD_TESTS          "Build tests"                                   OFF)
option(BUILD_DOCS           "Build documentation"                           OFF)
option(BUILD_UTILS          "Build utils executables"                       OFF)
option(BUILD_DEPS_LIBXML2   "Build libxml2 dependency"                      ON)
if(BUILD_TESTS)
    option(BUILD_DEPS_GTEST     "Build googletest dependency"                   ON)
    set(ENABLE_MEMHOOKS ON CACHE STRING "Enable compilation of memory hooks")
    set(TEST_MULTICAST_INTERFACE_IP "" CACHE STRING "Ip address of the interface to use for multicast when performing the tests")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    option(ED247_VERBOSE_DEBUG  "Enable verbose debug"                      ON)
else()
    option(ED247_VERBOSE_DEBUG  "Enable verbose debug"                      OFF)
endif()

# Set up the common directory structure for output libraries and binaries
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  # For Unix-like operating systems we can put all libraries into a dedicated
  # lib/ folder, whereas for Windows the libraries (dll files) need to be put
  # into the same directory as the executable, since Windows doesn't have an
  # RPATH equivalent where a search path can be put into the executable itself
  if(UNIX)
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  else()
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

if(MSVC)
    # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "lib")
    include(InstallRequiredSystemLibraries)

    # Prefix all shared libraries with 'lib'
    set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
    # Prefix all static libraries with 'lib'
    set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

# Dependencies
## LIBXML2
if(BUILD_DEPS_LIBXML2)
    if(MSVC)
        message(STATUS "Supress thirdparty warnings for windows platform")
        set(CMAKE_C_FLAGS_OLD "${CMAKE_C_FLAGS}")
        if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
          string(REGEX REPLACE "/W[0-4]" "/W0" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        else()
          set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W0")
        endif()
    endif()

    set(LIBXML2_WITH_ICONV OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_LZMA OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_PYTHON OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_ZLIB OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_TESTS OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_THREADS ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # TO TEST
    add_subdirectory(deps/libxml2 EXCLUDE_FROM_ALL)
    add_library(xml2 ALIAS LibXml2)
else()
    find_package(LibXml2 REQUIRED)
endif()
## GTEST
if(BUILD_DEPS_GTEST)
    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "My option" FORCE)
    endif()
    set(gtest_disable_pthreads OFF CACHE BOOL "My option" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "My option" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "My option" FORCE)
    set(GTEST_USE_OWN_TR1_TUPLE OFF CACHE BOOL "My option" FORCE)
    set(GTEST_HAS_TR1_TUPLE OFF CACHE BOOL "My option" FORCE)
    add_subdirectory(deps/googletest EXCLUDE_FROM_ALL)
else()
    if(BUILD_TESTS)
        find_package(GTest)
    endif()
endif()

if(UNIX)
    # if(BUILD_TESTS)
    #     message("-- Add coverage flags")
    #     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -ggdb")
    #     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -ggdb")
    #     set(CMAKE_EXE_LINKER "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    # endif()
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_SKIP_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../../lib:$ORIGIN/../lib")
endif()

# Add source folder
add_subdirectory(src)

if(BUILD_SAMPLES OR BUILD_TESTS OR BUILD_UTILS)
    add_subdirectory(shared)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

if(BUILD_UTILS)
    add_subdirectory(utils)
endif()

if(BUILD_DOCS)
    add_subdirectory(doc)
endif()

install(FILES LICENSE.md DESTINATION .)

