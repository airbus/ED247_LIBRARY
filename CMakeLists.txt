###############################################################################
# The MIT Licence                                                             #
#                                                                             #
# Copyright (c) 2021 Airbus Operations S.A.S                                  #
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
cmake_minimum_required(VERSION 3.22)
project(ED247_LIBRARY VERSION 1.1.3) # Define target version ${CMAKE_PROJECT_VERSION})
include(cmake/version.cmake)         # Compute ${VERSION} according to git
include(cmake/flags.cmake)

# Options multi-platform delivery.
if (PLATFORM_INSTALL_SUBDIR)
  if (NOT ${PLATFORM_INSTALL_SUBDIR} MATCHES "/$")
    set(PLATFORM_INSTALL_SUBDIR "${PLATFORM_INSTALL_SUBDIR}/")
  endif()
endif()
set(PLATFORM_INSTALL_SUBDIR "${PLATFORM_INSTALL_SUBDIR}" CACHE STRING "For multi-platform delivery. Sub folder where to install platform dependant binaries." FORCE)

# Options: test multicast ip.
if (NOT TEST_MULTICAST_INTERFACE_IP)
  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TEST_MULTICAST_INTERFACE_IP "127.0.0.1")
  endif()
endif()
set(TEST_MULTICAST_INTERFACE_IP "${TEST_MULTICAST_INTERFACE_IP}" CACHE STRING "Ip address of the interface to use for multicast when performing the tests.")


## LIBXML2
if (NOT LibXml2_ROOT)
  # Fix issue while building in 32bits
  set(LibXml2_ROOT /)
endif()
find_package(LibXml2 2.9.1 REQUIRED)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Fix iconv.h dependency
  find_package(Iconv QUIET)
  if (Iconv_FOUND)
    message("Using iconv header: ${Iconv_INCLUDE_DIRS}")
    target_include_directories(LibXml2::LibXml2 INTERFACE ${Iconv_INCLUDE_DIRS})
  endif()
endif()

# Simulink RT logger
add_library(SimulinkLogger INTERFACE)
find_path(SimulinkLogger_INCLUDE_DIR "Logger.hpp" HINTS ${SimulinkLogger_INCLUDE_DIR})
find_package_handle_standard_args(SimulinkLogger DEFAULT_MSG SimulinkLogger_INCLUDE_DIR)
if (SimulinkLogger_FOUND)
  target_include_directories(SimulinkLogger INTERFACE ${SimulinkLogger_INCLUDE_DIR})
  target_compile_definitions(SimulinkLogger INTERFACE "SIMULINK_LOGGER_ENABLED")
endif()


if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
  set(CMAKE_SHARED_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

# Add source folder
add_subdirectory(src)

add_subdirectory(shared)

add_subdirectory(samples)

add_subdirectory(utils)

# Enable tests only if GTest found
# To unsure find_package manage properly
# version information, we force the CONFIG mode.
find_package(GTest 1.8.1 CONFIG)

if (GTest_FOUND)
    # We set enable testing here instead of in tests cmake file 
    # because CTest need it in root directory.
    enable_testing()
    add_subdirectory(tests)
else()
    message(WARNING "GTest not found. The tests will not be available!")
endif()

# Doc
add_subdirectory(doc)

# Install 
install(FILES LICENSE.md DESTINATION doc)
set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

# Dump info
MESSAGE("# Configuration:")
MESSAGE("# Product version:        ${VERSION}")
MESSAGE("# System Name:            ${CMAKE_SYSTEM_NAME}")
MESSAGE("# Platform delivery dir:  ${PLATFORM_INSTALL_SUBDIR}")
MESSAGE("# LibXML2:                ${LIBXML2_LIBRARIES}")
MESSAGE("# GTEST:                  ${GTest_DIR}")
MESSAGE("# Doxygen:                ${DOXYGEN_EXECUTABLE}")
MESSAGE("")
