# Input variables:
#   ED247_LIB_DIR                 Default folder that contain ED247 library
#   TEST_NAME
#   WORKING_DIRECTORY             Where test executables are launched
#   TEST_NB_ACTORS                1 or 2
#   TEST_MULTICAST_INTERFACE_IP
message("Running test: ${TEST_NAME}")

# Allows to validate another library build (only for functional tests)
if (DEFINED ENV{ED247_LIB_DIR})
  set(ED247_LIB_DIR $ENV{ED247_LIB_DIR})
  message("Using ED247 library in '${ED247_LIB_DIR}'")
endif()

if (NOT DEFINED ED247_LIB_DIR OR NOT EXISTS ${ED247_LIB_DIR})
  message(FATAL_ERROR "Please set ED247_LIB_DIR to a valid path!")
endif()

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(ENV{PATH} "${ED247_LIB_DIR};$ENV{PATH}")
  set(CMAKE_EXECUTABLE_SUFFIX ".exe")
else()
  set(ENV{LD_LIBRARY_PATH} "${ED247_LIB_DIR};$ENV{LD_LIBRARY_PATH}")
endif()


if(TEST_NB_ACTORS EQUAL 1)
  execute_process(
    COMMAND "cmake"
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -DCMD_EXE=${WORKING_DIRECTORY}/${TEST_NAME}_main${CMAKE_EXECUTABLE_SUFFIX}
    -DCMD_ARG1=${WORKING_DIRECTORY}/config
    -DCMD_ARG2=${TEST_MULTICAST_INTERFACE_IP}
    -P ${CMAKE_CURRENT_LIST_DIR}/execute_process_redirect.cmake
    WORKING_DIRECTORY ${WORKING_DIRECTORY}
    COMMAND_ECHO STDOUT
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
    )
  if(NOT TEST_RESULT EQUAL 0)
    if("${TEST_ERROR}" STREQUAL "")
      message(FATAL_ERROR "Unknown error")
    else()
      message(FATAL_ERROR "${TEST_ERROR}")
    endif()
  endif()

elseif(TEST_NB_ACTORS EQUAL 2)
  execute_process(
    COMMAND "cmake"
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -DCMD_EXE=${WORKING_DIRECTORY}/${TEST_NAME}_sender${CMAKE_EXECUTABLE_SUFFIX}
    -DCMD_ARG1=${WORKING_DIRECTORY}/config
    -DCMD_ARG2=${TEST_MULTICAST_INTERFACE_IP}
    -P ${CMAKE_CURRENT_LIST_DIR}/execute_process_redirect.cmake
    COMMAND "cmake"
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -DCMD_EXE=${WORKING_DIRECTORY}/${TEST_NAME}_receiver${CMAKE_EXECUTABLE_SUFFIX}
    -DCMD_ARG1=${WORKING_DIRECTORY}/config
    -DCMD_ARG2=${TEST_MULTICAST_INTERFACE_IP}
    -P ${CMAKE_CURRENT_LIST_DIR}/execute_process_redirect.cmake
    WORKING_DIRECTORY ${WORKING_DIRECTORY}
    COMMAND_ECHO STDOUT
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
    )

  if(NOT TEST_RESULT EQUAL 0)
    if("${TEST_ERROR}" STREQUAL "")
      message(FATAL_ERROR "Unknown error")
    else()
      message(FATAL_ERROR "${TEST_ERROR}")
    endif()
  endif()

else()
  message(FATAL_ERROR "TEST_NB_ACTORS shall be 1 or 2 (not '${TEST_NB_ACTORS}')")
endif()
