###############################################################################
# The MIT Licence                                                             #
#                                                                             #
# Copyright (c) 2019 Airbus Operations S.A.S                                  #
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

CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
PROJECT(ED247_LIBRARY)

# Hardcoded options
SET(FT_PERFOS_MASTER_WITH_GPROF FALSE) # Enable compilation of FT_PERFOS_MASTER with GPROF compatible options
SET(LIBED247_VERBOSE_DEBUG FALSE) # Enable verbose debug logging

# Import dependencies (cots and components)
if(EXISTS "${CMAKE_SOURCE_DIR}/${VISUAL_GENERATED_CMAKE}" AND NOT IS_DIRECTORY "${CMAKE_SOURCE_DIR}/${VISUAL_GENERATED_CMAKE}")
    SET(GENERATED_DEPENDENCIES "${CMAKE_SOURCE_DIR}/${VISUAL_GENERATED_CMAKE}")
else()
    SET(GENERATED_DEPENDENCIES "${CMAKE_CURRENT_BINARY_DIR}/_dependencies.cmake")
endif()
if(NOT EXISTS "${GENERATED_DEPENDENCIES}")
    MESSAGE(FATAL_ERROR "The generated cmake dependency file cannot be found: ${GENERATED_DEPENDENCIES}")
endif()
INCLUDE(${GENERATED_DEPENDENCIES})

# Print some useful information to screen
STRING(TIMESTAMP CMAKE_CONFIGURE_DATE UTC)
MESSAGE("##############")
MESSAGE("# System Name [${CMAKE_SYSTEM_NAME}]")
MESSAGE("# Processor   [${CMAKE_SYSTEM_PROCESSOR}]")
MESSAGE("# Compiler    [${COMPILER}]")
MESSAGE("# Word size   [${WORDSIZE}]")
MESSAGE("# Project     [${PROJECT_NAME}]")
MESSAGE("# Directory   [${CMAKE_SOURCE_DIR}]")
MESSAGE("# Version     [${ED247_LIBRARY_VERSION}]")
MESSAGE("# Date        [${CMAKE_CONFIGURE_DATE}]")
MESSAGE("##############")

# Deal with exe dependencies
SET(XMLLINT_PROGRAM          ${LIBXML2_PATH}/bin/${XMLLINT_EXE})
if(NOT EXISTS "${XMLLINT_PROGRAM}")
    MESSAGE(FATAL_ERROR "Xmllint not found: ${XMLLINT_PROGRAM}")
endif()
SET(LCOV_PROGRAM             ${LCOV_PATH}/bin/${LCOV_EXE})
if((${ENABLE_LCOV}) AND (NOT EXISTS "${LCOV_PROGRAM}"))
    MESSAGE(FATAL_ERROR "Lcov not found: ${LCOV_PROGRAM}")
endif()
SET(GENHTML_PROGRAM          ${LCOV_PATH}/bin/${GENHTML_EXE})
if((${ENABLE_LCOV}) AND (NOT EXISTS "${GENHTML_PROGRAM}"))
    MESSAGE(FATAL_ERROR "Lcov genhtml not found: ${GENHTML_PROGRAM}")
endif()

# Deal with libraries dependencies
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/src/cmake")

# LIBXML2
IF(WIN32)
    FIND_PACKAGE(LibXml2 REQUIRED GLOBAL)
    SET(LIBXML2_INCLUDE_DIRS ${LIBXML2_INCLUDE_DIR})
ELSE(WIN32)
    FIND_PACKAGE(LibXml2 REQUIRED GLOBAL HINTS ${LIBXML2_PATH}/lib/cmake)
ENDIF(WIN32)

INCLUDE(GoogleTest)
SET(CMAKE_SIZEOF_VOID_P_MEM ${CMAKE_SIZEOF_VOID_P})
IF(${WORDSIZE} MATCHES "x86")
	SET(CMAKE_SIZEOF_VOID_P "4")
ENDIF(${WORDSIZE} MATCHES "x86")
FIND_PACKAGE(GTest REQUIRED GLOBAL HINTS ${GTEST_PATH}/lib64/cmake ${GTEST_PATH}/lib/cmake)
SET(CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P_MEM})

# Compilation flags and options
IF(COMPILER MATCHES "^msvc.*")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
    SET(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    SET(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
    SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "lib")
    INCLUDE(InstallRequiredSystemLibraries)
ELSE(COMPILER MATCHES "^msvc.*")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c11 -O0 -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -fvisibility=hidden")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -O0 -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -fvisibility=hidden")
    IF(WORDSIZE MATCHES "x86")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
    ENDIF(WORDSIZE MATCHES "x86")
    IF(COMPILER MATCHES "^gcc.*")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -rdynamic")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -rdynamic")
    ELSE()
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -static-libstdc++")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    ENDIF()
ENDIF(COMPILER MATCHES "^msvc.*")

IF(LIBED247_VERBOSE_DEBUG)
    MESSAGE("# Enable verboe debug mode")
    ADD_DEFINITIONS(-DLIBED247_VERBOSE_DEBUG)
ENDIF(LIBED247_VERBOSE_DEBUG)

# Install
SET(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../../lib")

# Subdirectories
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(doc)

INSTALL(FILES LICENSE.md DESTINATION .)

