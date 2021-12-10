# Static ED247 Library for unitary tests only
add_library(tests_unit_ed247_lib STATIC)

target_link_libraries(tests_unit_ed247_lib
    PUBLIC
        ed247_objects
        $<$<PLATFORM_ID:Windows>:wsock32>
        $<$<PLATFORM_ID:Windows>:ws2_32>
)

# Testing API
add_library(test_api OBJECT test_context.cpp memhooks.cpp)

target_include_directories(test_api
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/src/ed247
)

target_link_libraries(test_api
  PUBLIC
    sync_entity
    $<TARGET_OBJECTS:sync_entity>
    GTest::gtest
)
