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

get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
string(TOLOWER ${CURRENT_DIR_NAME} TEST_EXECUTABLE)

add_executable(${TEST_EXECUTABLE}_main src/${CURRENT_DIR_NAME}_main.cpp)
add_executable(${TEST_EXECUTABLE}_tester src/${CURRENT_DIR_NAME}_tester.cpp)

target_link_libraries(${TEST_EXECUTABLE}_main
    PUBLIC ed247 sync_entity test_context
    PRIVATE $<$<BOOL:${QNXNTO}>:regex>)
target_link_libraries(${TEST_EXECUTABLE}_tester
    PUBLIC ed247 sync_entity test_context
    PRIVATE $<$<BOOL:${QNXNTO}>:regex>)

install(TARGETS ${TEST_EXECUTABLE}_main ${TEST_EXECUTABLE}_tester
    RUNTIME DESTINATION tests/bin)

file(GLOB CONFIGS LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/config/*)
foreach(CONFIG ${CONFIGS})
    get_filename_component(CONFIG_FILENAME ${CONFIG} NAME)
    configure_file(${CONFIG} ${CMAKE_BINARY_DIR}/config/${CONFIG_FILENAME} @ONLY)
    install(FILES ${CMAKE_BINARY_DIR}/config/${CONFIG_FILENAME} DESTINATION tests/config)
endforeach()

add_test(NAME ${TEST_EXECUTABLE}
    COMMAND "cmake"
            -DTEST=${TEST_EXECUTABLE}
            -DWORKING_DIRECTORY=$<TARGET_FILE_DIR:${TEST_EXECUTABLE}_main>
            -DSEND_EXE=$<TARGET_FILE:${TEST_EXECUTABLE}_main>
            -DSEND_CONFIG=${CMAKE_BINARY_DIR}/config
            -DRECV_EXE=$<TARGET_FILE:${TEST_EXECUTABLE}_tester>
            -DRECV_CONFIG=${CMAKE_BINARY_DIR}/config
    -P ${CMAKE_SOURCE_DIR}/cmake/runtest.cmake)