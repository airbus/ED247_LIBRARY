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
include(${CMAKE_CURRENT_LIST_DIR}/cmake/generate_ecic.cmake)

get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
string(TOLOWER ${CURRENT_DIR_NAME} TEST_EXECUTABLE)

add_executable(${TEST_EXECUTABLE}_send src/${CURRENT_DIR_NAME}_send.cpp)
add_executable(${TEST_EXECUTABLE}_recv src/${CURRENT_DIR_NAME}_recv.cpp)
add_custom_target(${TEST_EXECUTABLE}
    DEPENDS
        ${TEST_EXECUTABLE}_send
        ${TEST_EXECUTABLE}_recv
)
add_dependencies(tests ${TEST_EXECUTABLE})

target_link_libraries(${TEST_EXECUTABLE}_send libtests_functional)
target_link_libraries(${TEST_EXECUTABLE}_recv libtests_functional)

set(CONFIG_SEND_FILEPATH ${CMAKE_BINARY_DIR}/config/ecic_${TEST_EXECUTABLE}_send.xml)
generate_ecic(${CONFIG_SEND_FILEPATH} "Out" 10 100)

set(CONFIG_RECV_FILEPATH ${CMAKE_BINARY_DIR}/config/ecic_${TEST_EXECUTABLE}_recv.xml)
generate_ecic(${CONFIG_RECV_FILEPATH} "In" 10 100)

add_test(NAME ${TEST_EXECUTABLE}
    COMMAND "cmake"
        -DED247_LIB_PATH=$<TARGET_FILE_DIR:ed247>
        -DTEST=${TEST_EXECUTABLE}
        -DWORKING_DIRECTORY=$<TARGET_FILE_DIR:${TEST_EXECUTABLE}_send>
        -DSEND_EXE=$<TARGET_FILE:${TEST_EXECUTABLE}_send>
        -DSEND_CONFIG=${CMAKE_BINARY_DIR}/config
        -DRECV_EXE=$<TARGET_FILE:${TEST_EXECUTABLE}_recv>
        -DRECV_CONFIG=${CMAKE_BINARY_DIR}/config
        -P ${CMAKE_SOURCE_DIR}/tests/cmake/execute_test.cmake)
