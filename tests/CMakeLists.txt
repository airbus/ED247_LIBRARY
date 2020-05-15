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

# The second set of applications uses the test api
# The test api is not compiled for coverage
ADD_SUBDIRECTORY(api)

IF(NOT MSVC)
# # For all the remaining builds, the coverage must be used
SET(CMAKE_C_FLAGS_OLD           "${CMAKE_C_FLAGS}")
SET(CMAKE_C_LINK_FLAGS_OLD      "${CMAKE_C_LINK_FLAGS}")
SET(CMAKE_CXX_FLAGS_OLD         "${CMAKE_CXX_FLAGS}")
SET(CMAKE_CXX_LINK_FLAGS_OLD    "${CMAKE_CXX_LINK_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS_OLD  "${CMAKE_EXE_LINKER_FLAGS}")
ENDIF()

ADD_SUBDIRECTORY(FT_PERFOS)

IF(NOT MSVC)
# For all the remaining builds, the coverage must be used
SET(CMAKE_C_FLAGS               "${CMAKE_C_FLAGS_OLD} --coverage -ggdb")
SET(CMAKE_C_LINK_FLAGS          "${CMAKE_C_LINK_FLAGS_OLD} --coverage -ggdb")
SET(CMAKE_CXX_FLAGS             "${CMAKE_CXX_FLAGS_OLD} --coverage -ggdb")
SET(CMAKE_CXX_LINK_FLAGS        "${CMAKE_CXX_LINK_FLAGS_OLD} --coverage -ggdb")
SET(CMAKE_EXE_LINKER_FLAGS      "${CMAKE_EXE_LINKER_FLAGS_OLD} --coverage")
ENDIF()

# A common test lib is built with the exact same sources as the delivered library
ADD_SUBDIRECTORY(test_lib)

# A first set of tests uses api functions for functionnal validation
ADD_SUBDIRECTORY(FT_BACKUP)
ADD_SUBDIRECTORY(FT_EXCHANGE)
ADD_SUBDIRECTORY(FT_EXCHANGE_DATATIMESTAMP)
ADD_SUBDIRECTORY(FT_FRAME)
ADD_SUBDIRECTORY(FT_LIMIT)
ADD_SUBDIRECTORY(FT_LOAD_ALL)
ADD_SUBDIRECTORY(FT_LOGGING)
ADD_SUBDIRECTORY(UT_API_CHANNELS)
ADD_SUBDIRECTORY(UT_API_CONVERTERS)
ADD_SUBDIRECTORY(UT_API_METRICS)
ADD_SUBDIRECTORY(UT_API_MISC)
ADD_SUBDIRECTORY(UT_API_SIGNALS)
ADD_SUBDIRECTORY(UT_API_STREAMS)
ADD_SUBDIRECTORY(UT_API_USER_FEEDBACK)

# A second set of tests uses internal structures of the library to perform unit tests
ADD_SUBDIRECTORY(UT_CHANNELS)
ADD_SUBDIRECTORY(UT_LOADING)
ADD_SUBDIRECTORY(UT_SIGNALS)
ADD_SUBDIRECTORY(UT_SOCKETS)
ADD_SUBDIRECTORY(UT_STREAMS)
ADD_SUBDIRECTORY(UT_TIMESTAMP)

# A third set of tests related to the resolver.
ADD_SUBDIRECTORY(FT_EXCHANGE_EXTERNAL)

# This first set of applications uses only the original library
# They consist in simple examples without test assesments
ADD_SUBDIRECTORY(EXAMPLE_WALK_CONFIGURATION)
ADD_SUBDIRECTORY(EXAMPLE_STREAM_SEND_RECV_WAIT_FRAME)
ADD_SUBDIRECTORY(EXAMPLE_STREAM_SEND_RECV_WAIT_DURING)
ADD_SUBDIRECTORY(EXAMPLE_STREAM_SEND_RECV_EVENT)
ADD_SUBDIRECTORY(EXAMPLE_SIGNAL_SEND_RECV_WAIT_FRAME)
