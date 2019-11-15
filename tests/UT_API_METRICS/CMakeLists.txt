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

GET_FILENAME_COMPONENT(CURRENT_DIR_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${CURRENT_DIR_NAME} TEST_EXECUTABLE)

SET(CMAKE_VERBOSE_MAKEFILE ON)

MESSAGE("## TEST [${CMAKE_PROJECT_NAME}]:[${TEST_EXECUTABLE}]")

ADD_EXECUTABLE(${TEST_EXECUTABLE}_send ${TEST_API_SOURCES} src/${CURRENT_DIR_NAME}_send.cpp)
ADD_EXECUTABLE(${TEST_EXECUTABLE}_recv ${TEST_API_SOURCES} src/${CURRENT_DIR_NAME}_recv.cpp)

# Add dependencies
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_send ed247_test)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_recv ed247_test)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_send PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_recv PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247)

TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_send GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_recv GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_send PUBLIC ${GTEST_PATH}/include)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_recv PUBLIC ${GTEST_PATH}/include)

TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_send test_api)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_recv test_api)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_send PUBLIC ${TEST_API_INCLUDE_DIR})
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_recv PUBLIC ${TEST_API_INCLUDE_DIR})


# Define deployed items
INSTALL(TARGETS ${TEST_EXECUTABLE}_send 
    RUNTIME DESTINATION tests/bin)
INSTALL(TARGETS ${TEST_EXECUTABLE}_recv
    RUNTIME DESTINATION tests/bin)

FILE(GLOB CONFIGS LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/config/*)
FILE(GLOB SH LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/sh/*)

INSTALL(FILES ${CONFIGS}
    DESTINATION tests/config/${TEST_EXECUTABLE})
INSTALL(FILES ${SH}
    DESTINATION tests/bin
    PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
