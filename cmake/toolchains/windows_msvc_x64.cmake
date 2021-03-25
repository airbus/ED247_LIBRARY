set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)

# DEBUG
# set(CMAKE_EXE_LINKER_DEBUG_INIT "/NODEFAULTLIB:MSVCRT")

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)