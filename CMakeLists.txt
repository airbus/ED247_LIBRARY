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
    # option(ENABLE_MEMHOOKS      "Enable compilation of memory hooks"            ON)
    set(TEST_MULTICAST_INTERFACE_IP "" CACHE STRING "Ip address of the interface to use for multicast when performing the tests")
else()
    # option(ENABLE_MEMHOOKS      "Enable compilation of memory hooks"            OFF)
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
endif()

# Dependencies
## LIBXML2
if(BUILD_DEPS_LIBXML2)
    set(LIBXML2_WITH_ICONV OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_LZMA OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_PYTHON OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_ZLIB OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_TESTS OFF CACHE BOOL "" FORCE)
    set(LIBXML2_WITH_THREADS ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # TO TEST
    add_subdirectory(deps/libxml2 EXCLUDE_FROM_ALL)
else()
    find_package(LibXml2)
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

# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
#     set(CMAKE_EXE_LINKER "${CMAKE_EXE_LINKER} ${CMAKE_EXE_LINKER_DEBUG}")
# else()
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
#     set(CMAKE_EXE_LINKER "${CMAKE_EXE_LINKER} ${CMAKE_EXE_LINKER_RELEASE}")
# endif()

# Add compilation flags
# set(CMAKE_CXX_STANDARD 11)
if(MSVC)

else()
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wno-narrowing")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-narrowing")
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -fvisibility=hidden")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -fvisibility=hidden")
    # if(WIN32)
    #     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
    #     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
    # else()
    #     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -rdynamic")
    #     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -rdynamic")
    # endif()
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

# set(CMAKE_SKIP_BUILD_RPATH FALSE)
# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../../lib:$ORIGIN/../../src/ed247")

# CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
# PROJECT(ED247_LIBRARY)

# # Hardcoded options
# SET(FT_PERFOS_MASTER_WITH_GPROF FALSE) # Enable compilation of FT_PERFOS_MASTER with GPROF compatible options
# SET(LIBED247_VERBOSE_DEBUG FALSE) # Enable verbose debug logging

# # Import dependencies (cots and components)
# if(EXISTS "${CMAKE_SOURCE_DIR}/${VISUAL_GENERATED_CMAKE}" AND NOT IS_DIRECTORY "${CMAKE_SOURCE_DIR}/${VISUAL_GENERATED_CMAKE}")
#     SET(GENERATED_DEPENDENCIES "${CMAKE_SOURCE_DIR}/${VISUAL_GENERATED_CMAKE}")
# else()
#     SET(GENERATED_DEPENDENCIES "${CMAKE_CURRENT_BINARY_DIR}/_dependencies.cmake")
# endif()
# if(NOT EXISTS "${GENERATED_DEPENDENCIES}")
#     MESSAGE(FATAL_ERROR "The generated cmake dependency file cannot be found: ${GENERATED_DEPENDENCIES}")
# endif()
# INCLUDE(${GENERATED_DEPENDENCIES})

# # Print some useful information to screen
# STRING(TIMESTAMP CMAKE_CONFIGURE_DATE UTC)
# MESSAGE("##############")
# MESSAGE("# System Name [${CMAKE_SYSTEM_NAME}]")
# MESSAGE("# Processor   [${CMAKE_SYSTEM_PROCESSOR}]")
# MESSAGE("# Compiler    [${COMPILER}]")
# MESSAGE("# Word size   [${WORDSIZE}]")
# MESSAGE("# Project     [${PROJECT_NAME}]")
# MESSAGE("# Directory   [${CMAKE_SOURCE_DIR}]")
# MESSAGE("# Version     [${ED247_LIBRARY_VERSION}]")
# MESSAGE("# Date        [${CMAKE_CONFIGURE_DATE}]")
# MESSAGE("##############")

# # Deal with exe dependencies
# SET(XMLLINT_PROGRAM          ${LIBXML2_PATH}/bin/${XMLLINT_EXE})
# if(NOT EXISTS "${XMLLINT_PROGRAM}")
#     MESSAGE(FATAL_ERROR "Xmllint not found: ${XMLLINT_PROGRAM}")
# endif()
# SET(LCOV_PROGRAM             ${LCOV_PATH}/bin/${LCOV_EXE})
# if((${ENABLE_LCOV}) AND (NOT EXISTS "${LCOV_PROGRAM}"))
#     MESSAGE(FATAL_ERROR "Lcov not found: ${LCOV_PROGRAM}")
# endif()
# SET(GENHTML_PROGRAM          ${LCOV_PATH}/bin/${GENHTML_EXE})
# if((${ENABLE_LCOV}) AND (NOT EXISTS "${GENHTML_PROGRAM}"))
#     MESSAGE(FATAL_ERROR "Lcov genhtml not found: ${GENHTML_PROGRAM}")
# endif()

# # Deal with libraries dependencies
# SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/src/cmake")

# # LIBXML2
# IF(WIN32)
#     FIND_PACKAGE(LibXml2 REQUIRED GLOBAL)
#     SET(LIBXML2_INCLUDE_DIRS ${LIBXML2_INCLUDE_DIR})
# ELSE(WIN32)
#     FIND_PACKAGE(LibXml2 REQUIRED GLOBAL HINTS ${LIBXML2_PATH}/lib/cmake)
# ENDIF(WIN32)

# INCLUDE(GoogleTest)
# SET(CMAKE_SIZEOF_VOID_P_MEM ${CMAKE_SIZEOF_VOID_P})
# IF(${WORDSIZE} MATCHES "x86")
# 	SET(CMAKE_SIZEOF_VOID_P "4")
# ENDIF(${WORDSIZE} MATCHES "x86")
# FIND_PACKAGE(GTest REQUIRED GLOBAL HINTS ${GTEST_PATH}/lib64/cmake ${GTEST_PATH}/lib/cmake)
# SET(CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P_MEM})

# # Compilation flags and options
# IF(COMPILER MATCHES "^msvc.*")
#     SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
#     SET(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
#     SET(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
#     SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "lib")
#     INCLUDE(InstallRequiredSystemLibraries)
# ELSE(COMPILER MATCHES "^msvc.*")
#     SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c11 -O0 -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -fvisibility=hidden")
#     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -O0 -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -fvisibility=hidden")
#     IF(WORDSIZE MATCHES "x86")
#         SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
#         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
#         SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
#     ENDIF(WORDSIZE MATCHES "x86")
#     IF(COMPILER MATCHES "^gcc.*")
#         SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -rdynamic")
#         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -rdynamic")
#     ELSE()
#         SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -static-libstdc++")
#         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
#     ENDIF()
# ENDIF(COMPILER MATCHES "^msvc.*")

# IF(LIBED247_VERBOSE_DEBUG)
#     MESSAGE("# Enable verboe debug mode")
#     ADD_DEFINITIONS(-DLIBED247_VERBOSE_DEBUG)
# ENDIF(LIBED247_VERBOSE_DEBUG)

# # Install
# SET(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../../lib")

# # Subdirectories
# ADD_SUBDIRECTORY(src)
# ADD_SUBDIRECTORY(tests)
# ADD_SUBDIRECTORY(doc)

# INSTALL(FILES LICENSE.md DESTINATION .)

