add_library(libsynchronizer OBJECT synchronizer.cpp)
target_include_directories(libsynchronizer
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
