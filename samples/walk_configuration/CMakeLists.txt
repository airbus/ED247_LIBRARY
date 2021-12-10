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

add_executable(${TEST_EXECUTABLE} src/${CURRENT_DIR_NAME}_main.c)

target_link_libraries(${TEST_EXECUTABLE} PRIVATE Ed247::ed247)

file(GLOB CONFIGS LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/config/*)
install(FILES ${CONFIGS} DESTINATION doc/samples/${TEST_EXECUTABLE}/config)

file(GLOB SRCS LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/src/*)
install(FILES ${SRCS} DESTINATION doc/samples/${TEST_EXECUTABLE}/src)
