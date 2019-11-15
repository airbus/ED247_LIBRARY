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

UNSET(SRCS)
FILE(GLOB SRCS LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

ADD_LIBRARY(memhooks SHARED ${SRCS})
SET_TARGET_PROPERTIES(memhooks PROPERTIES OUTPUT_NAME memhooks)
TARGET_INCLUDE_DIRECTORIES(memhooks PUBLIC ${CMAKE_CURRENT_LIST_DIR})

UNSET(SRCS)

IF(WIN32)
    TARGET_COMPILE_DEFINITIONS(memhooks PRIVATE MEMHOOKS_EXPORTS)
    TARGET_COMPILE_DEFINITIONS(memhooks PRIVATE _WINDOWS)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(memhooks PROPERTIES PUBLIC_HEADER memhooks.h)

INSTALL(TARGETS memhooks
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib)