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

# Input variables
#   TEST                Test name
#   WORKING_DIRECTORY   Where test executables are launched
#   SEND_EXE            First executable
#   SEND_CONFIG         First configuration file
#   RECV_EXE            Second executable
#   RECV_CONFIG         Second configuration file

message("Running test: ${TEST}")

# Set PATH to libED247 on windows (rpath is used on Linux)
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  if (ED247_LIB_PATH)
    set(ENV{PATH} "${ED247_LIB_PATH};$ENV{PATH}")
  endif()
endif()

if(RUN_EXE)

  execute_process(
    COMMAND "cmake"
    -DRUN_EXE=${RUN_EXE}
    -DRUN_CONFIG=${RUN_CONFIG}
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -P ${CMAKE_CURRENT_LIST_DIR}/execute_process_redirect.cmake
    WORKING_DIRECTORY ${WORKING_DIRECTORY}
    COMMAND_ECHO STDOUT
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
    )
  if(NOT TEST_RESULT EQUAL 0)
    if("${TEST_ERROR}" STREQUAL "")
      message(FATAL_ERROR "Unknown error")
    else()
      message(FATAL_ERROR "${TEST_ERROR}")
    endif()
  endif()

else()
  execute_process(
    COMMAND "cmake"
    -DRUN_EXE=${SEND_EXE}
    -DRUN_CONFIG=${SEND_CONFIG}
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -P ${CMAKE_CURRENT_LIST_DIR}/execute_process_redirect.cmake
    COMMAND "cmake"
    -DRUN_EXE=${RECV_EXE}
    -DRUN_CONFIG=${RECV_CONFIG}
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -P ${CMAKE_CURRENT_LIST_DIR}/execute_process_redirect.cmake
    WORKING_DIRECTORY ${WORKING_DIRECTORY}
    COMMAND_ECHO STDOUT
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
    )

  if(NOT TEST_RESULT EQUAL 0)
    if("${TEST_ERROR}" STREQUAL "")
      message(FATAL_ERROR "Unknown error")
    else()
      message(FATAL_ERROR "${TEST_ERROR}")
    endif()
  endif()

endif()
