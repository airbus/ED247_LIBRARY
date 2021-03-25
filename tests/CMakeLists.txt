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

add_subdirectory(memhooks)
add_subdirectory(test_context)

add_subdirectory(func_load_all)
add_subdirectory(func_rev0_compliance)
add_subdirectory(func_exchange)
add_subdirectory(func_exchange_datatimestamp)
add_subdirectory(func_frame)
add_subdirectory(func_limit)
add_subdirectory(func_logging)

add_subdirectory(unit_api_channels)
add_subdirectory(unit_api_converters)
add_subdirectory(unit_api_metrics)
add_subdirectory(unit_api_misc)
add_subdirectory(unit_api_signals)
add_subdirectory(unit_api_streams)
add_subdirectory(unit_api_user_feedback)
add_subdirectory(unit_channels)
add_subdirectory(unit_loading)
add_subdirectory(unit_signals)
add_subdirectory(unit_sockets)
add_subdirectory(unit_streams)
add_subdirectory(unit_timestamp)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/../doc/tests/report.md "
# Test report

| Test | Status |
|:----:|:------:|
")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/../doc/tests/report.md DESTINATION doc/tests)
