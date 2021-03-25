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

include(${CMAKE_CURRENT_LIST_DIR}/generate_ecic.cmake)

set(OUTPUT ON CACHE BOOL "Whether the generated ECIC file is made of input streams.")

if(OUTPUT)
    set(DIRECTION "Out")
else()
    set(DIRECTION "In")
endif()

set(FILEPATH "ecic.xml" CACHE STRING "Filepath of the generated ECIC file.")
set(NUM_CHANNELS "1" CACHE STRING "Number of channels.")
set(NUM_STREAMS "1" CACHE STRING "Number of streams.")
set(NUM_SIGNALS "1" CACHE STRING "Number of signals.")
set(STREAM_TYPE "A429_Stream" CACHE STRING "Type of stream.")
set(TEST_MULTICAST_INTERFACE_IP "" CACHE STRING "Multicast interface IP address.")

generate_ecic(${FILEPATH} ${DIRECTION} ${NUM_CHANNELS} ${NUM_STREAMS} ${NUM_SIGNALS} ${STREAM_TYPE})