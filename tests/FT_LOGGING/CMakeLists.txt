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

MESSAGE("## TEST [${CMAKE_PROJECT_NAME}]:[${TEST_EXECUTABLE}]")

# Define executable
ADD_EXECUTABLE(${TEST_EXECUTABLE} src/${CURRENT_DIR_NAME}_actor.cpp)
ADD_EXECUTABLE(${TEST_EXECUTABLE}_invalid_logfile src/${CURRENT_DIR_NAME}_invalid_logfile.cpp)

# Add dependencies
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE} ed247_test)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_invalid_logfile ed247_test)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_invalid_logfile PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247)

TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE} GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_invalid_logfile GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE} PUBLIC ${GTEST_PATH}/include)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_invalid_logfile PUBLIC ${GTEST_PATH}/include)

TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE} test_api)
TARGET_LINK_LIBRARIES(${TEST_EXECUTABLE}_invalid_logfile test_api)
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE} PUBLIC ${TEST_API_INCLUDE_DIR})
TARGET_INCLUDE_DIRECTORIES(${TEST_EXECUTABLE}_invalid_logfile PUBLIC ${TEST_API_INCLUDE_DIR})

# Define deployed items
INSTALL(TARGETS ${TEST_EXECUTABLE}
    RUNTIME DESTINATION tests/bin)
INSTALL(TARGETS ${TEST_EXECUTABLE}_invalid_logfile
    RUNTIME DESTINATION tests/bin)

FILE(GLOB CONFIGS LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/config/*)
FILE(GLOB SH LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/sh/*)

INSTALL(FILES ${CONFIGS}
    DESTINATION tests/config/${TEST_EXECUTABLE})
INSTALL(FILES ${SH}
    DESTINATION tests/bin
    PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
