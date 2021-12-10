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
get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
string(TOLOWER ${CURRENT_DIR_NAME} TEST_EXECUTABLE)

add_executable(${TEST_EXECUTABLE} src/${CURRENT_DIR_NAME}_main.cpp)
add_dependencies(tests ${TEST_EXECUTABLE})

target_link_libraries(${TEST_EXECUTABLE} PRIVATE test_api tests_unit_ed247_lib)

add_test(NAME ${TEST_EXECUTABLE}
    COMMAND "cmake"
        -DTEST=${TEST_EXECUTABLE}
        -DWORKING_DIRECTORY=$<TARGET_FILE_DIR:${TEST_EXECUTABLE}>
        -DRUN_EXE=$<TARGET_FILE:${TEST_EXECUTABLE}>
        -DRUN_CONFIG=${TEST_MULTICAST_INTERFACE_IP} # Give the mc interface ip as argument
        -P ${CMAKE_SOURCE_DIR}/tests/cmake/execute_test.cmake)
