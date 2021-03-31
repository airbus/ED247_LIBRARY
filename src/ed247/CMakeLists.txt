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

# Append source files
unset(SRCS)
file(GLOB SRCS LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
if(ENABLE_MEMHOOKS)
    file(GLOB SRCS_MEMHOOKS ${CMAKE_CURRENT_LIST_DIR}/memhooks/*.cpp)
    list(APPEND SRCS ${SRCS_MEMHOOKS})
endif()
add_library(ed247o OBJECT ${SRCS})
unset(SRCS)

# Append include directories
target_include_directories(ed247o PUBLIC ${CMAKE_CURRENT_LIST_DIR})
if(ENABLE_MEMHOOKS)
    target_include_directories(ed247o PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../memhooks)
    target_include_directories(ed247o PUBLIC ${CMAKE_CURRENT_LIST_DIR}/memhooks)
endif()

# Append definitions
target_compile_definitions(ed247o PUBLIC "_PRODUCT_VERSION=\"${ED247_LIBRARY_VERSION}\"")
target_compile_definitions(ed247o PUBLIC "_PRODUCT_NAME=\"${CMAKE_PROJECT_NAME}\"")
if(WIN32)
    target_compile_definitions(ed247o PUBLIC LIBED247_EXPORTS)
endif()
if(ENABLE_MEMHOOKS)
    target_compile_definitions(ed247o PUBLIC ENABLE_MEMHOOKS)
endif()

# Append link to other libraries
target_link_libraries(ed247o LibXml2)
if(WIN32)
    target_link_libraries(ed247o wsock32 ws2_32)
endif()
if(ENABLE_MEMHOOKS)
    target_link_libraries(ed247o memhooks)
endif()

# Define shared and static targets
add_library(ed247 SHARED $<TARGET_OBJECTS:ed247o>)
target_link_libraries(ed247 LibXml2)
if(ENABLE_MEMHOOKS)
    target_link_libraries(ed247 memhooks)
endif()
target_include_directories(ed247 PUBLIC ${CMAKE_CURRENT_LIST_DIR})
# set_target_properties(ed247 PROPERTIES PUBLIC_HEADER ed247.h)
add_library(ed247_static STATIC $<TARGET_OBJECTS:ed247o>)

# Install required stuff
install(FILES ${CMAKE_CURRENT_LIST_DIR}/ed247.h DESTINATION inc)
install(TARGETS ed247
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib)
install(TARGETS ed247_static
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib)

# Generate XSD header file containing the XSD as a string
macro(_generate_xsd_header _XSD_PATH _XSD_FILENAME _HEADER_FILEPATH _VAR_NAME)
    file(READ ${_XSD_PATH}/${_XSD_FILENAME} XSD_CONTENT)
    string(REPLACE "\\" "\\\\" XSD_CONTENT ${XSD_CONTENT})
    string(REPLACE "\"" "\\\"" XSD_CONTENT ${XSD_CONTENT})
    string(REPLACE "\n" "\\n\"\n\"" XSD_CONTENT ${XSD_CONTENT})
    string(TIMESTAMP GENERATION_TIME)
    file(WRITE ${_HEADER_FILEPATH} "// File generated from <${_XSD_FILENAME}> at ${GENERATION_TIME}
static const char* ${_VAR_NAME} =
\"${XSD_CONTENT}\\n\";
")
endmacro(_generate_xsd_header _XSD_PATH _XSD_FILENAME _HEADER_FILEPATH _VAR_NAME)

_generate_xsd_header(${CMAKE_CURRENT_LIST_DIR}/xsd ED247A_ECIC.xsd ${CMAKE_CURRENT_LIST_DIR}/ed247_xsd.h xsd_schema)

install(FILES ./xsd/ED247A_ECIC.xsd DESTINATION xsd)
install(FILES ./xsd/ED247A_ECD.xsd DESTINATION xsd)