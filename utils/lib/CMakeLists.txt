add_library(libutils OBJECT a429_tools.cpp)
target_include_directories(libutils
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
