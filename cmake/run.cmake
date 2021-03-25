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

message("RUN: ${RUN_EXE} ${RUN_CONFIG}")
if(WIN32)
    string(REPLACE ".exe" ".out" RUN_LOG ${RUN_EXE})
    string(REPLACE ".exe" ".err" RUN_ERR ${RUN_EXE})
else()
    set(RUN_LOG "${RUN_EXE}.log")
    set(RUN_ERR "${RUN_EXE}.err")
endif()

if(UNIX)
    set(ENV{LD_PRELOAD} "${WORKING_DIRECTORY}/../lib/libmemhooks.so")
    message("LD_PRELOAD $ENV{LD_PRELOAD}")
    set(ENV{MEMHOOKS_LEVEL} 2)
    message("MEMHOOKS_LEVEL $ENV{MEMHOOKS_LEVEL}")
endif()

execute_process(
    COMMAND ${RUN_EXE} ${RUN_CONFIG}
    WORKING_DIRECTORY ${WORKING_DIRECTORY}
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
    OUTPUT_FILE ${RUN_LOG}
    ERROR_FILE ${RUN_ERR}
)
if(NOT TEST_RESULT EQUAL 0)
    if("${TEST_ERROR}" STREQUAL "")
        message(FATAL_ERROR "ERROR\nUnknown error")
    else()
        message(FATAL_ERROR "ERROR\n${TEST_ERROR}")
    endif()
endif()