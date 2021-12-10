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
if(WIN32)
  string(REPLACE ".exe" ".log" RUN_LOG ${RUN_EXE})
else()
  set(RUN_LOG "${RUN_EXE}.log")
endif()

execute_process(
  COMMAND ${RUN_EXE} ${RUN_CONFIG}
  WORKING_DIRECTORY ${WORKING_DIRECTORY}
  RESULT_VARIABLE TEST_RESULT
  COMMAND_ECHO STDOUT
  OUTPUT_FILE ${RUN_LOG}
  ERROR_FILE ${RUN_LOG}
  )
if(NOT TEST_RESULT EQUAL 0)
  message(FATAL_ERROR "Test failed! : Result: ${TEST_RESULT} Log file: ${RUN_LOG}")
else()
  message(STATUS "Test successed! Log file: ${RUN_LOG}")
endif()
