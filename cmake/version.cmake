#
# Create the VERSION variable set to current git tag
# Has fallback if git not available or not under control
#
unset(VERSION CACHE)

find_package(Git)
if(Git_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty RESULT_VARIABLE STATUS OUTPUT_VARIABLE VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(STATUS AND NOT STATUS EQUAL 0)
    unset(VERSION)
  endif()
endif()

if (NOT VERSION)
  if (CMAKE_PROJECT_VERSION)
    set(VERSION ${CMAKE_PROJECT_VERSION})
  else()
    get_filename_component(SOURCE_BASENAME ${CMAKE_SOURCE_DIR} NAME)
    set(VERSION "${SOURCE_BASENAME}-noSCM")
  endif()
  message("Cannot get version from GIT scm! Set to default value: '${VERSION}'")
endif()
