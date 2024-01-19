# Base tests library
add_library(libtests_objects OBJECT memhooks.cpp)
target_include_directories(libtests_objects
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(libtests_objects
  PUBLIC
  GTest::gtest
)

# Test library statically linked to ED247
add_library(libtests_static STATIC tests_tools.cpp)
target_compile_definitions(libtests_static
  PUBLIC
    $<$<PLATFORM_ID:Windows>:LIBED247_STATIC>
)
target_link_libraries(libtests_static
  PUBLIC
    Ed247::static
    libtests_objects
    libsynchronizer
    libtime_tools
    $<$<PLATFORM_ID:Linux>:dl>
)

# Test library dynamically linked to ED247
add_library(libtests_dynamic STATIC tests_tools.cpp)

target_link_libraries(libtests_dynamic
  PUBLIC
    Ed247::ed247
    libtests_objects
    libsynchronizer
    libtime_tools
    $<$<PLATFORM_ID:Linux>:dl>
)
