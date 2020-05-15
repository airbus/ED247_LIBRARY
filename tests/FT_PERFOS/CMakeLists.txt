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

GET_FILENAME_COMPONENT(CURRENT_DIR_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${CURRENT_DIR_NAME} TEST_EXECUTABLE)

MESSAGE("## TEST [${CMAKE_PROJECT_NAME}]:[${TEST_EXECUTABLE}]")

##

FILE(GLOB_RECURSE SRCLIBFILES LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247/*.cpp)
ADD_LIBRARY(ed247_test_tmp STATIC ${SRCMEMHKSFILES} ${SRCLIBFILES})

TARGET_COMPILE_DEFINITIONS(ed247_test_tmp PUBLIC "LIBED247_VERSION=\"${LIBED247_VERSION}\"")
TARGET_COMPILE_DEFINITIONS(ed247_test_tmp PUBLIC "LIBED247_NAME=\"${CMAKE_PROJECT_NAME}\"")

TARGET_INCLUDE_DIRECTORIES(ed247_test_tmp PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    TARGET_LINK_LIBRARIES(ed247_test_tmp wsock32 ws2_32)
    TARGET_COMPILE_DEFINITIONS(ed247_test_tmp PUBLIC LIBED247_EXPORTS)
    TARGET_COMPILE_DEFINITIONS(ed247_test_tmp PUBLIC _WINDOWS)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

TARGET_LINK_LIBRARIES(ed247_test_tmp memhooks)
TARGET_INCLUDE_DIRECTORIES(ed247_test_tmp PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/memhooks)

FIND_LIBRARY(LIBXML2_STATIC_LIBRARIES libxml2.a libxml2_a libxml2_a_dll xml2_a xml2_a_dll HINTS ${LIBXML2_LIBRARY_DIR})
TARGET_LINK_LIBRARIES(ed247_test_tmp ${LIBXML2_STATIC_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(ed247_test_tmp PUBLIC ${LIBXML2_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

## Slave

ADD_EXECUTABLE(${TEST_EXECUTABLE}_slave ${TEST_API_SOURCES} src/${CURRENT_DIR_NAME}_slave.cpp)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_slave ed247_test_tmp)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_slave PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_slave GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_slave PUBLIC ${GTEST_PATH}/include)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_slave test_api)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_slave PUBLIC ${TEST_API_INCLUDE_DIR})
INSTALL(TARGETS ${TEST_EXECUTABLE}_slave
    RUNTIME DESTINATION tests/bin)

IF(NOT MSVC AND FT_PERFOS_MASTER_WITH_GPROF)
MESSAGE("### COMPILE MASTER FOR GPROF USE")
SET(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS} -pg -fno-pie --coverage -ggdb")
SET(CMAKE_C_LINK_FLAGS      "${CMAKE_C_LINK_FLAGS} -pg -fno-pie --coverage -ggdb")
SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -pg -fno-pie --coverage -ggdb")
SET(CMAKE_CXX_LINK_FLAGS    "${CMAKE_CXX_LINK_FLAGS} -pg -fno-pie --coverage -ggdb")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pg -fno-pie --coverage -ggdb")
ENDIF()

## Master

ADD_EXECUTABLE(${TEST_EXECUTABLE}_master ${TEST_API_SOURCES} src/${CURRENT_DIR_NAME}_master.cpp)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_master ed247_test_tmp)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_master PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_master GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_master PUBLIC ${GTEST_PATH}/include)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_master test_api)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_master PUBLIC ${TEST_API_INCLUDE_DIR})
INSTALL(TARGETS ${TEST_EXECUTABLE}_master
    RUNTIME DESTINATION tests/bin)

## Common

FILE(GLOB SH LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/sh/*)

INSTALL(FILES ${SH}
    DESTINATION tests/bin
    PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
