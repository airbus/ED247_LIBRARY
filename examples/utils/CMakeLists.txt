add_library(utils STATIC
  version.cpp
  traces.cpp
  sync.cpp
  a429_tools.cpp)
target_compile_definitions(utils PRIVATE "_PRODUCT_VERSION=${VERSION}")
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(utils PUBLIC ${LIBED247_INCLUDE_DIR})
