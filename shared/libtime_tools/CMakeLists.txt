add_library(libtime_tools OBJECT time_tools.cpp)
target_include_directories(libtime_tools
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
