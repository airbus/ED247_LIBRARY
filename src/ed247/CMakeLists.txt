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
# Special object target that will also be used by unitary tests
add_library(ed247_objects OBJECT
    ed247_logs.cpp
    ed247_time.cpp
    ed247_conversion.cpp
    ed247_xml.cpp
    ed247_cominterface.cpp
    ed247_sample.cpp
    ed247_signal.cpp
    ed247_stream_assistant.cpp
    ed247_stream.cpp
    ed247_frame_header.cpp
    ed247_channel.cpp
    ed247_context.cpp
    ed247.cpp
)

target_link_libraries(ed247_objects
  PUBLIC
     LibXml2::LibXml2
     $<$<PLATFORM_ID:Windows>:wsock32>
     $<$<PLATFORM_ID:Windows>:ws2_32>
     $<$<PLATFORM_ID:QNX>:socket>
     $<$<PLATFORM_ID:QNX>:regex>
     $<$<BOOL:${SimulinkLogger_FOUND}>:SimulinkLogger>
)

target_include_directories(ed247_objects
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
     $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
     $<INSTALL_INTERFACE:include>
)

target_compile_definitions(ed247_objects
    PRIVATE
        "_PRODUCT_VERSION=${VERSION}"
        "_PRODUCT_NAME=${CMAKE_PROJECT_NAME}"
        $<$<PLATFORM_ID:Windows>:LIBED247_EXPORTS>
)

# ED247 Library definition
add_library(ed247 SHARED)
add_library(Ed247::ed247 ALIAS ed247)
target_link_libraries(ed247 PUBLIC ed247_objects)
set_target_properties(ed247 PROPERTIES PUBLIC_HEADER ed247.h)

# Generate XSD header file containing the XSD as a string
macro(_generate_xsd_header _XSD_PATH _XSD_FILENAME _HEADER_FILEPATH _VAR_NAME)
  if(${_XSD_PATH}/${_XSD_FILENAME} IS_NEWER_THAN ${_HEADER_FILEPATH})
    message("Generate ${_HEADER_FILEPATH}...")
    file(READ ${_XSD_PATH}/${_XSD_FILENAME} XSD_CONTENT)
    string(REPLACE "\\" "\\\\" XSD_CONTENT ${XSD_CONTENT})
    string(REPLACE "\"" "\\\"" XSD_CONTENT ${XSD_CONTENT})
    string(REPLACE "\n" "\\n\"\n\"" XSD_CONTENT ${XSD_CONTENT})
    string(TIMESTAMP GENERATION_TIME)
    file(WRITE ${_HEADER_FILEPATH} "// File generated from <${_XSD_FILENAME}> at ${GENERATION_TIME}
static const char* ${_VAR_NAME} =
\"${XSD_CONTENT}\\n\";
")
  endif()
endmacro(_generate_xsd_header _XSD_PATH _XSD_FILENAME _HEADER_FILEPATH _VAR_NAME)

_generate_xsd_header(${CMAKE_CURRENT_LIST_DIR}/xsd ED247A_ECIC.xsd ${CMAKE_CURRENT_BINARY_DIR}/ed247_xsd.h xsd_schema)

# Install required stuff
set(INSTALL_LIBDIR lib)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(INSTALL_LIBDIR bin)
endif()

install(TARGETS ed247
                                     DESTINATION ${PLATFORM_INSTALL_SUBDIR}${INSTALL_LIBDIR}
  PUBLIC_HEADER                      DESTINATION ${PLATFORM_INSTALL_SUBDIR}include
)
install(FILES ./xsd/ED247A_ECIC.xsd  DESTINATION doc/xsd)
install(FILES ./xsd/ED247A_ECD.xsd   DESTINATION doc/xsd)

