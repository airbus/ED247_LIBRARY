set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS_INIT "-Wall -Wextra -fvisibility=hidden")
set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -fvisibility=hidden")

# DEBUG
set(CMAKE_C_FLAGS_DEBUG_INIT "-O0 -g -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g -ggdb")

# RELEASE
set(CMAKE_C_FLAGS_RELEASE_INIT "-g")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-g")

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)