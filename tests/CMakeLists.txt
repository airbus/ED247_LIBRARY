add_subdirectory(libtests EXCLUDE_FROM_ALL)
add_custom_target(tests_functional)
add_custom_target(tests_unitary)
add_custom_target(tests DEPENDS tests_functional tests_unitary)

#
# Add a target ${_test_basename}_config to generate ECIC files for a test
#
function (test_add_config _test_basename)
  set(config_target_list "")
  file(GLOB config_template_list LIST_DIRECTORIES false ${_test_basename}/config/*)
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
  add_custom_target(${_test_basename}_config DEPENDS ${config_target_list})
endfunction()

#
# Add an executable to a test
# ${_test_basename}
# ${_link_type} : either static or dynamic
# ${_exe_suffix} : main, sender, receiver, ...
#
function (test_add_executable _test_basename _link_type _exe_suffix)
  set(exe_name ${_test_basename}_${_link_type}_${_exe_suffix})
  add_executable(${exe_name} EXCLUDE_FROM_ALL ${_test_basename}/src/${_test_basename}_${_exe_suffix}.cpp)
  set_target_properties(${exe_name} PROPERTIES SKIP_BUILD_RPATH TRUE)
  target_compile_definitions(${exe_name} PRIVATE "TEST_PRODUCT_VERSION=\"${VERSION}\"")
  target_link_libraries(${exe_name} libtests_${_link_type})
  add_dependencies(${_test_basename}_${_link_type} ${exe_name})
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
# Add a test
# ${_test_basename}
# ${_nb_actors} : 1 or 2
# ${_link_type} : either static or dynamic
#
function(test_create _test_basename _nb_actors _link_type)
  add_custom_target(${_test_basename}_${_link_type})
  add_dependencies(${_test_basename}_${_link_type} ${_test_basename}_config)

  if(_nb_actors EQUAL 1)
    test_add_executable(${_test_basename} ${_link_type} main)
  else()
    test_add_executable(${_test_basename} ${_link_type} sender)
    test_add_executable(${_test_basename} ${_link_type} receiver)
  endif()

  test_declare_ctest(${_test_basename}_${_link_type} ${_nb_actors})
endfunction()


#
# Add all tests for _test_basename
# ${_test_basename}
# ${_nb_actors} : 1 or 2
# ${_kind} : either functional or unitary
#
function(test_create_all _test_basename _nb_actors _kind)
  test_add_config(${_test_basename})
  add_custom_target(${_test_basename})

  test_create(${_test_basename} ${_nb_actors} static)
  add_dependencies(tests_${_kind} ${_test_basename}_static)

  if(_kind STREQUAL "functional")
    test_create(${_test_basename} ${_nb_actors} dynamic)
    add_dependencies(tests_${_kind} ${_test_basename}_dynamic)
  endif()

  # Run dynamic test by default
  if(_kind STREQUAL "functional")
    add_dependencies(${_test_basename} ${_test_basename}_dynamic)
    add_custom_target(run_tests_${_test_basename} ${CMAKE_CTEST_COMMAND} -R "${_test_basename}_dynamic$$" --output-on-failure DEPENDS ${_test_basename}_dynamic)
  else()
    add_dependencies(${_test_basename} ${_test_basename}_static)
    add_custom_target(run_tests_${_test_basename} ${CMAKE_CTEST_COMMAND} -R "${_test_basename}_static$$" --output-on-failure DEPENDS ${_test_basename}_static)
  endif()

endfunction()

#
# Add a test with a single actor
# ${_test_basename}
# ${_kind} : either functional or unitary
# Actor source file shall be ${_test_basename}/src/${_test_basename}_main.cpp
#
function(test_create_for_one_actor _test_basename _kind)
  test_create_all(${_test_basename} 1 ${_kind})
endfunction()

#
# Add a test with two actors
# ${_test_basename}
# ${_kind} : either functional or unitary
# Actor source files shall be :
#  * ${_test_basename}/src/${_test_basename}_sender.cpp
#  * ${_test_basename}/src/${_test_basename}_receiver.cpp
#
function(test_create_for_two_actors _test_basename _kind)
  test_create_all(${_test_basename} 2 ${_kind})
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
test_create_for_two_actors(func_synchro_delay            functional)
test_create_for_two_actors(func_streamassistant_helpers  functional)

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

# Handle test results
if (PLATFORM_ID STREQUAL "")
  set(TEST_RESULTS_BASENAME test_results)
else()
  set(TEST_RESULTS_BASENAME test_results_${PLATFORM_ID})
endif()
set(TEST_RESULTS_TXT ${CMAKE_BINARY_DIR}/tests/${TEST_RESULTS_BASENAME}.txt)
set(TEST_RESULTS_XML ${CMAKE_BINARY_DIR}/tests/${TEST_RESULTS_BASENAME}.xml)

if(NOT EXISTS ${TEST_RESULTS_TXT})
  file(WRITE ${TEST_RESULTS_TXT} "No tests has been ran.\n")
endif()

install(FILES ${TEST_RESULTS_TXT} DESTINATION doc)

# run_tests target
add_custom_target(run_tests_functional ${CMAKE_CTEST_COMMAND} -R '^func_' --output-on-failure DEPENDS tests_functional)
add_custom_target(run_tests_unitary ${CMAKE_CTEST_COMMAND} -R '^unit_' --output-on-failure DEPENDS tests_unitary)

if(EXISTS ${CMAKE_SOURCE_DIR}/doc/airbus)
  include(${CMAKE_SOURCE_DIR}/doc/airbus/cmake/run_tests.cmake)
else()
  add_custom_target(run_tests ${CMAKE_CTEST_COMMAND} -O ${TEST_RESULTS_TXT} --output-junit ${TEST_RESULTS_XML} --output-on-failure DEPENDS tests)
endif()
