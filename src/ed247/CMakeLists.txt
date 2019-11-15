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

FILE(GLOB_RECURSE SRCS LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

ADD_LIBRARY(ed247 SHARED ${SRCS})
ADD_LIBRARY(ed247_static STATIC ${SRCS})
TARGET_INCLUDE_DIRECTORIES(ed247 PUBLIC ${CMAKE_CURRENT_LIST_DIR})
TARGET_INCLUDE_DIRECTORIES(ed247_static PUBLIC ${CMAKE_CURRENT_LIST_DIR})

UNSET(SRCS)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    TARGET_LINK_LIBRARIES(ed247 wsock32 ws2_32)
    TARGET_LINK_LIBRARIES(ed247_static wsock32 ws2_32)
    TARGET_COMPILE_DEFINITIONS(ed247 PUBLIC LIBED247_EXPORTS)
    TARGET_COMPILE_DEFINITIONS(ed247_static PUBLIC LIBED247_EXPORTS)
ENDIF()

TARGET_COMPILE_DEFINITIONS(ed247 PUBLIC "LIBED247_VERSION=\"${LIBED247_VERSION}\"")
TARGET_COMPILE_DEFINITIONS(ed247_static PUBLIC "LIBED247_VERSION=\"${LIBED247_VERSION}\"")
TARGET_COMPILE_DEFINITIONS(ed247 PUBLIC "LIBED247_NAME=\"${CMAKE_PROJECT_NAME}\"")
TARGET_COMPILE_DEFINITIONS(ed247_static PUBLIC "LIBED247_NAME=\"${CMAKE_PROJECT_NAME}\"")

# LIBXML2 (static)
MESSAGE("## LIBXML2_LIBRARY ${LIBXML2_LIBRARY}")
MESSAGE("## LIBXML2_LIBRARIES ${LIBXML2_LIBRARIES}")
MESSAGE("## LIBXML2_LIBRARY_DIR ${LIBXML2_LIBRARY_DIR}")
MESSAGE("## LIBXML2_INCLUDE_DIRS ${LIBXML2_INCLUDE_DIRS}")
MESSAGE("## LIBXML2_INSTALL_PREFIX ${LIBXML2_INSTALL_PREFIX}")
IF(WIN32)
    GET_FILENAME_COMPONENT(LIBXML2_LIBRARY_DIR ${LIBXML2_LIBRARY} DIRECTORY)
ENDIF(WIN32)
FIND_LIBRARY(LIBXML2_STATIC_LIBRARIES libxml2.a libxml2_a libxml2_a_dll xml2_a xml2_a_dll HINTS ${LIBXML2_LIBRARY_DIR})
MESSAGE("### LIBXML2_STATIC_LIBRARIES ${LIBXML2_STATIC_LIBRARIES}")
TARGET_LINK_LIBRARIES(ed247 ${LIBXML2_LIBRARIES})
TARGET_LINK_LIBRARIES(ed247_static ${LIBXML2_STATIC_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(ed247 PUBLIC ${LIBXML2_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})
TARGET_INCLUDE_DIRECTORIES(ed247_static PUBLIC ${LIBXML2_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

# Memhooks
TARGET_LINK_LIBRARIES(ed247 memhooks)
TARGET_LINK_LIBRARIES(ed247_static memhooks)
TARGET_INCLUDE_DIRECTORIES(ed247 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../memhooks)
TARGET_INCLUDE_DIRECTORIES(ed247_static PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../memhooks)

SET_TARGET_PROPERTIES(ed247 PROPERTIES PUBLIC_HEADER ed247.h)
# Headers are the same: deploy once for both binaries !
#SET_TARGET_PROPERTIES(ed247_static PROPERTIES PUBLIC_HEADER ed247.h)

INSTALL(TARGETS ed247 ed247_static
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
    PUBLIC_HEADER DESTINATION inc)

INSTALL(FILES ${LIBXML2_LIBRARY} DESTINATION lib)

# Generate XSD header file containing the XSD as a string
MACRO(_GENERATE_XSD_HEADER _XSD_PATH _XSD_FILENAME _HEADER_FILEPATH _VAR_NAME)
    FILE(READ ${_XSD_PATH}/${_XSD_FILENAME} XSD_CONTENT)
    STRING(REPLACE "\\" "\\\\" XSD_CONTENT ${XSD_CONTENT})
    STRING(REPLACE "\"" "\\\"" XSD_CONTENT ${XSD_CONTENT})
    STRING(REPLACE "\n" "\\n\"\n\"" XSD_CONTENT ${XSD_CONTENT})
    STRING(TIMESTAMP GENERATION_TIME)
    FILE(WRITE ${_HEADER_FILEPATH} "// File generated from <${_XSD_FILENAME}> at ${GENERATION_TIME}
static const char* ${_VAR_NAME} =
\"${XSD_CONTENT}\\n\";
")
ENDMACRO(_GENERATE_XSD_HEADER _XSD_PATH _XSD_FILENAME _HEADER_FILEPATH _VAR_NAME)

INSTALL(FILES ./xsd/ED247A_ECIC.xsd DESTINATION xsd)
INSTALL(FILES ./xsd/ED247A_ECD.xsd DESTINATION xsd)

_GENERATE_XSD_HEADER(${CMAKE_CURRENT_LIST_DIR}/xsd ED247A_ECIC.xsd ${CMAKE_CURRENT_LIST_DIR}/ed247_xsd.h xsd_schema)