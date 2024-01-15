#
# Set compiler flags
#

# Debug by default
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

# Warning level
if(MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
endif()

# C++-11
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 11)
