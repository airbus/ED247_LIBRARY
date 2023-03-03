add_subdirectory(libtests EXCLUDE_FROM_ALL)
add_custom_target(tests_functional)
add_custom_target(tests_unitary)
add_custom_target(tests DEPENDS tests_functional tests_unitary)

#
# Add a target to generate ECIC files for a test
#
function (test_add_config _test_name)
  set(config_target_list "")
  file(GLOB config_template_list LIST_DIRECTORIES false ${_test_name}/config/*)
  foreach(config_template ${config_template_list})
    get_filename_component(config_filename ${config_template} NAME)
    set(config_target ${CMAKE_CURRENT_BINARY_DIR}/config/${config_filename})
    add_custom_command(
      OUTPUT     ${config_target}
      COMMAND    ${CMAKE_COMMAND}
        -D INFILE=${config_template}
        -D OUTFILE=${config_target}
        -D TEST_MULTICAST_INTERFACE_IP=${TEST_MULTICAST_INTERFACE_IP}
        -P ${CMAKE_CURRENT_LIST_DIR}/cmake/configure_file.cmake
      DEPENDS    ${config_template}
      )
    list(APPEND config_target_list ${config_target})
  endforeach()
  add_custom_target(${_test_name}_config DEPENDS ${config_target_list})
  add_dependencies(${_test_name} ${_test_name}_config)
endfunction()

#
# Add an executable to a test
# ${_test_name}
# ${_kind} : either functional or unitary
# ${_exe_suffix} : main, sender, receiver, ...
#
function (test_add_executable _test_name _kind _exe_suffix)
  set(exe_name ${_test_name}_${_exe_suffix})
  add_executable(${exe_name} EXCLUDE_FROM_ALL ${_test_name}/src/${exe_name}.cpp)
  set_target_properties(${exe_name} PROPERTIES SKIP_BUILD_RPATH TRUE)
  target_compile_definitions(${exe_name} PRIVATE "TEST_PRODUCT_VERSION=\"${VERSION}\"")
  target_link_libraries(${exe_name} libtests_${_kind})
  add_dependencies(${_test_name} ${exe_name})
endfunction()


#
# declare a test to CTEST
# ${_test_name}
# ${_nb_actors} : 1 or 2
#
function (test_declare_ctest _test_name _nb_actors)
  add_test(
    NAME ${_test_name}
    COMMAND "cmake"
      -DTEST_NAME=${_test_name}
      -DED247_LIB_DIR=$<TARGET_FILE_DIR:ed247>
      -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
      -DTEST_NB_ACTORS=${_nb_actors}
      -DTEST_MULTICAST_INTERFACE_IP=${TEST_MULTICAST_INTERFACE_IP}
      -P ${CMAKE_SOURCE_DIR}/tests/cmake/execute_test.cmake
  )
endfunction()



#
# Add a test with a single actor
# ${_test_name}
# ${_kind} : either functional or unitary
# Actor source file shall be ${_test_name}/src/${_test_name}_main.cpp
#
function(test_create_for_one_actor _test_name _kind)
  add_custom_target(${_test_name})
  add_dependencies(tests_${_kind} ${_test_name})

  test_add_config(${_test_name})
  test_add_executable(${_test_name} ${_kind} main)

  test_declare_ctest(${_test_name} 1)
endfunction()

#
# Add a test with two actors
# ${_test_name}
# ${_kind} : either functional or unitary
# Actor source files shall be :
#  * ${_test_name}/src/${_test_name}_sender.cpp
#  * ${_test_name}/src/${_test_name}_receiver.cpp
#
function(test_create_for_two_actors _test_name _kind)
  add_custom_target(${_test_name})
  add_dependencies(tests_${_kind} ${_test_name})

  test_add_config(${_test_name})
  test_add_executable(${_test_name} ${_kind} sender)
  test_add_executable(${_test_name} ${_kind} receiver)

  test_declare_ctest(${_test_name} 2)
endfunction()

# Functional tests
test_create_for_one_actor(func_load_all                  functional)
test_create_for_one_actor(func_logging                   functional)

test_create_for_two_actors(func_rev0_compliance          functional)
test_create_for_two_actors(func_exchange                 functional)
test_create_for_two_actors(func_exchange_datatimestamp   functional)
test_create_for_two_actors(func_exchange_tts             functional)
test_create_for_two_actors(func_exchange_streamassistant functional)
test_create_for_two_actors(func_limit                    functional)
test_create_for_two_actors(func_metrics                  functional)
test_create_for_two_actors(func_application_fifo         functional)
test_create_for_two_actors(func_error_handling           functional)
test_create_for_two_actors(func_complex_multichannel     functional)
test_create_for_two_actors(func_bidirectional            functional)
test_create_for_two_actors(func_multi_address            functional)

# Unitary tests
test_create_for_one_actor(unit_api_channels            unitary)
test_create_for_one_actor(unit_api_converters          unitary)
test_create_for_one_actor(unit_api_misc                unitary)
test_create_for_one_actor(unit_api_signals             unitary)
test_create_for_one_actor(unit_api_streams             unitary)
test_create_for_one_actor(unit_api_user_feedback       unitary)
test_create_for_one_actor(unit_channels                unitary)
test_create_for_one_actor(unit_loading                 unitary)
test_create_for_one_actor(unit_signals                 unitary)
test_create_for_one_actor(unit_sockets                 unitary)
test_create_for_one_actor(unit_streams                 unitary)

# Handle test reports
if (PLATFORM_ID STREQUAL "")
  set(TEST_REPORT_BASENAME tests_report)
else()
  set(TEST_REPORT_BASENAME tests_report_${PLATFORM_ID})
endif()
set(TEST_REPORT_TXT ${CMAKE_BINARY_DIR}/tests/${TEST_REPORT_BASENAME}.txt)
set(TEST_REPORT_XML ${CMAKE_BINARY_DIR}/tests/${TEST_REPORT_BASENAME}.xml)

if(NOT EXISTS ${TEST_REPORT_TXT})
  file(WRITE ${TEST_REPORT_TXT} "No tests has been ran.\n")
endif()

install(FILES ${TEST_REPORT_TXT} DESTINATION doc)

# run_tests target
add_custom_target(run_tests_functional ${CMAKE_CTEST_COMMAND} -R '^func_' --output-on-failure DEPENDS tests_functional)
add_custom_target(run_tests_unitary ${CMAKE_CTEST_COMMAND} -R '^unit_' --output-on-failure DEPENDS tests_unitary)

if(EXISTS ${CMAKE_SOURCE_DIR}/doc/airbus)
  include(${CMAKE_SOURCE_DIR}/doc/airbus/cmake/run_tests.cmake)
else()
  add_custom_target(run_tests ${CMAKE_CTEST_COMMAND} -O ${TEST_REPORT_TXT} --output-junit ${TEST_REPORT_XML} --output-on-failure DEPENDS tests)
endif()
