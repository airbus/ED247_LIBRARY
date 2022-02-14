get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
string(TOLOWER ${CURRENT_DIR_NAME} TEST_EXECUTABLE)

add_executable(${TEST_EXECUTABLE}_sender src/${CURRENT_DIR_NAME}_sender.cpp)
add_executable(${TEST_EXECUTABLE}_receiver src/${CURRENT_DIR_NAME}_receiver.cpp)
add_custom_target(${TEST_EXECUTABLE}
    DEPENDS
        ${TEST_EXECUTABLE}_sender
        ${TEST_EXECUTABLE}_receiver
)
add_dependencies(tests ${TEST_EXECUTABLE})

target_link_libraries(${TEST_EXECUTABLE}_sender libtests_functional)
target_link_libraries(${TEST_EXECUTABLE}_receiver libtests_functional)

file(GLOB CONFIGS LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/config/*)
foreach(CONFIG ${CONFIGS})
    get_filename_component(CONFIG_FILENAME ${CONFIG} NAME)
    configure_file(${CONFIG} ${CMAKE_BINARY_DIR}/config/${CONFIG_FILENAME} @ONLY)
endforeach()

add_test(NAME ${TEST_EXECUTABLE}
    COMMAND "cmake"
            -DED247_LIB_PATH=$<TARGET_FILE_DIR:ed247>
            -DTEST=${TEST_EXECUTABLE}
            -DWORKING_DIRECTORY=$<TARGET_FILE_DIR:${TEST_EXECUTABLE}_sender>
            -DSEND_EXE=$<TARGET_FILE:${TEST_EXECUTABLE}_sender>
            -DSEND_CONFIG=${CMAKE_BINARY_DIR}/config
            -DRECV_EXE=$<TARGET_FILE:${TEST_EXECUTABLE}_receiver>
            -DRECV_CONFIG=${CMAKE_BINARY_DIR}/config
            -P ${CMAKE_SOURCE_DIR}/tests/cmake/execute_test.cmake)
