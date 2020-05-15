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

# Create a testing facility library to prevent recompilation for each test
UNSET(SRCS)

SET(TEST_API_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
FILE(GLOB_RECURSE SRCS LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
ADD_LIBRARY(test_api OBJECT ${SRCS})
TARGET_INCLUDE_DIRECTORIES(test_api PUBLIC ${TEST_API_INCLUDE_DIR})

UNSET(SRCS)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    TARGET_LINK_LIBRARIES(test_api wsock32 ws2_32)
    TARGET_COMPILE_DEFINITIONS(test_api PUBLIC LIBED247_EXPORTS)
ENDIF()

IF(WIN32)
    TARGET_COMPILE_DEFINITIONS(test_api PUBLIC TEST_EXPORTS)
    TARGET_COMPILE_DEFINITIONS(test_api PUBLIC _WINDOWS)
ENDIF(WIN32)

TARGET_INCLUDE_DIRECTORIES(test_api PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/memhooks)
TARGET_INCLUDE_DIRECTORIES(test_api PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../src/ed247)

TARGET_LINK_LIBRARIES(test_api GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
TARGET_INCLUDE_DIRECTORIES(test_api PUBLIC ${GTEST_PATH}/include)

# Also deploy environment files
IF(MSVC)

    CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/env/test.sh.in ${CMAKE_CURRENT_BINARY_DIR}/env/test.sh @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/env/test.sh
        DESTINATION tests
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
		
    CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/env/iotest.sh.in ${CMAKE_CURRENT_BINARY_DIR}/env/iotest.sh @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/env/iotest.sh
        DESTINATION tests
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/env/run_test_msvc.bat.in ${CMAKE_CURRENT_BINARY_DIR}/env/run_test.bat @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/env/run_test.bat
        DESTINATION tests
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

ELSEIF(MINGW)

    CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/env/test.sh.in ${CMAKE_CURRENT_BINARY_DIR}/env/test.sh @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/env/test.sh
        DESTINATION tests
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
		
    CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/env/iotest.sh.in ${CMAKE_CURRENT_BINARY_DIR}/env/iotest.sh @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/env/iotest.sh
        DESTINATION tests
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/env/run_test_mingw.bat.in ${CMAKE_CURRENT_BINARY_DIR}/env/run_test.bat @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/env/run_test.bat
        DESTINATION tests
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/env/setup_test_env.sh.in ${CMAKE_CURRENT_BINARY_DIR}/env/setup_test_env.sh @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/env/setup_test_env.sh
        DESTINATION tests
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        
ELSE()

    CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/env/test.sh.in ${CMAKE_CURRENT_BINARY_DIR}/env/test.sh @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/env/test.sh
        DESTINATION tests
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/env/iotest.sh.in ${CMAKE_CURRENT_BINARY_DIR}/env/iotest.sh @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/env/iotest.sh
        DESTINATION tests
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/env/setup_test_env.sh.in ${CMAKE_CURRENT_BINARY_DIR}/env/setup_test_env.sh @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/env/setup_test_env.sh
        DESTINATION tests
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

ENDIF()