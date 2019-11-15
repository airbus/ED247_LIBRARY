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

# Create a library that will be used for test purposes only

FILE(GLOB_RECURSE SRCLIBFILES LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247/*.cpp)
ADD_LIBRARY(ed247_test STATIC ${SRCMEMHKSFILES} ${SRCLIBFILES})

TARGET_COMPILE_DEFINITIONS(ed247_test PUBLIC "LIBED247_VERSION=\"${LIBED247_VERSION}\"")
TARGET_COMPILE_DEFINITIONS(ed247_test PUBLIC "LIBED247_NAME=\"${CMAKE_PROJECT_NAME}\"")

TARGET_INCLUDE_DIRECTORIES(ed247_test PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    TARGET_LINK_LIBRARIES(ed247_test wsock32 ws2_32)
    TARGET_COMPILE_DEFINITIONS(ed247_test PUBLIC LIBED247_EXPORTS)
    TARGET_COMPILE_DEFINITIONS(ed247_test PUBLIC _WINDOWS)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

IF(NOT MSVC)
TARGET_LINK_LIBRARIES(ed247_test gcov)
ENDIF()

TARGET_LINK_LIBRARIES(ed247_test memhooks)
TARGET_INCLUDE_DIRECTORIES(ed247_test PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/memhooks)

FIND_LIBRARY(LIBXML2_STATIC_LIBRARIES libxml2.a libxml2_a libxml2_a_dll xml2_a xml2_a_dll HINTS ${LIBXML2_LIBRARY_DIR})
TARGET_LINK_LIBRARIES(ed247_test ${LIBXML2_STATIC_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(ed247_test PUBLIC ${LIBXML2_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

