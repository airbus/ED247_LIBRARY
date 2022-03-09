# Input variables:
#   TEST_NAME
#   WORKING_DIRECTORY             Where test executables are launched
#   TEST_NB_ACTORS                1 or 2
#   TEST_MULTICAST_INTERFACE_IP

message("Running test: ${TEST_NAME}")

# Set PATH to libED247 on windows (rpath is used on Linux)
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  if (ED247_LIB_PATH)
    set(ENV{PATH} "${ED247_LIB_PATH};$ENV{PATH}")
  endif()
endif()

if(TEST_NB_ACTORS EQUAL 1)
  execute_process(
    COMMAND "cmake"
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -DCMD_EXE=${WORKING_DIRECTORY}/${TEST_NAME}_main
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
    -DCMD_EXE=${WORKING_DIRECTORY}/${TEST_NAME}_sender
    -DCMD_ARG1=${WORKING_DIRECTORY}/config
    -DCMD_ARG2=${TEST_MULTICAST_INTERFACE_IP}
    -P ${CMAKE_CURRENT_LIST_DIR}/execute_process_redirect.cmake
    COMMAND "cmake"
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -DCMD_EXE=${WORKING_DIRECTORY}/${TEST_NAME}_receiver
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
