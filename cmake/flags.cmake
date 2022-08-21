#
# Set compiler flags
#

# Debug by default
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

# Prevent symbols to be exported by defaut
if(NOT MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

# Warning level
if(MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

# C++-11
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 11)

# We are building a library
# TODO: this flag shal not be global as is it not needed for binaries (tools, tests, ...)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_CROSSCOMPILING AND CMAKE_HOST_WIN32)
  # Windows does not support RPATH
  set(CMAKE_SKIP_RPATH TRUE)
endif()

# Report unresolved symbol references as errors
if(NOT MSVC)
  string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--no-undefined")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -Wl,--no-undefined")
endif()