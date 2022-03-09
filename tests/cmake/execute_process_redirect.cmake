# Input variables:
# CMD_EXE
# CMD_ARG1 CMD_ARG2
# WORKING_DIRECTORY
#

if(WIN32)
  string(REPLACE ".exe" ".log" CMD_LOG ${CMD_EXE})
else()
  set(CMD_LOG "${CMD_EXE}.log")
endif()

execute_process(
  COMMAND ${CMD_EXE} ${CMD_ARG1} ${CMD_ARG2}
  WORKING_DIRECTORY ${WORKING_DIRECTORY}
  RESULT_VARIABLE TEST_RESULT
  COMMAND_ECHO STDOUT
  OUTPUT_FILE ${CMD_LOG}
  ERROR_FILE ${CMD_LOG}
  )
if(NOT TEST_RESULT EQUAL 0)
  message(FATAL_ERROR "Test failed! : Result: ${TEST_RESULT} Log file: ${CMD_LOG}")
else()
  message(STATUS "Test successed! Log file: ${CMD_LOG}")
endif()
