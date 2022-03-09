add_subdirectory(libtests)
add_custom_target(tests)


#
# Generate ECIC files for ${_test_name}
#
function (test_generate_config _test_name)
  file(GLOB config_file_list LIST_DIRECTORIES false ${_test_name}/config/*)
  foreach(config_file ${config_file_list})
    get_filename_component(config_filename ${config_file} NAME)
    configure_file(${config_file} ${CMAKE_CURRENT_BINARY_DIR}/config/${config_filename} @ONLY)
  endforeach()
endfunction()

#
# Add an executable to a test
# ${_test_name}
# ${kind} : either functional or unitary
# ${_exe_suffix} : main, sender, receiver, ...
#
function (test_add_executable _test_name kind _exe_suffix)
  set(exe_name ${_test_name}_${_exe_suffix})
  add_executable(${exe_name} ${_test_name}/src/${exe_name}.cpp)
  target_compile_definitions(${exe_name} PRIVATE "TEST_PRODUCT_VERSION=\"${VERSION}\"")
  target_link_libraries(${exe_name} libtests_${kind})
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
      -DED247_LIB_PATH=$<TARGET_FILE_DIR:ed247>
      -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
      -DTEST_NB_ACTORS=${_nb_actors}
      -DTEST_MULTICAST_INTERFACE_IP=${TEST_MULTICAST_INTERFACE_IP}
      -P ${CMAKE_SOURCE_DIR}/tests/cmake/execute_test.cmake
  )
endfunction()



#
# Add a test with a single actor
# ${_test_name}
# ${kind} : either functional or unitary
# Actor source file shall be ${_test_name}/src/${_test_name}_main.cpp
#
function(test_create_for_one_actor _test_name kind)
  add_custom_target(${_test_name})
  add_dependencies(tests ${_test_name})
  test_generate_config(${_test_name})

  test_add_executable(${_test_name} ${kind} main)

  test_declare_ctest(${_test_name} 1)
endfunction()

#
# Add a test with two actors
# ${_test_name}
# ${kind} : either functional or unitary
# Actor source files shall be :
#  * ${_test_name}/src/${_test_name}_sender.cpp
#  * ${_test_name}/src/${_test_name}_receiver.cpp
#
function(test_create_for_two_actors _test_name kind)
  add_custom_target(${_test_name})
  add_dependencies(tests ${_test_name})
  test_generate_config(${_test_name})

  test_add_executable(${_test_name} ${kind} sender)
  test_add_executable(${_test_name} ${kind} receiver)

  test_declare_ctest(${_test_name} 2)
endfunction()

# Functional tests
test_create_for_one_actor(func_frame                   functional)
test_create_for_one_actor(func_load_all                functional)
test_create_for_one_actor(func_logging                 functional)

test_create_for_two_actors(func_rev0_compliance        functional)
test_create_for_two_actors(func_exchange               functional)
test_create_for_two_actors(func_exchange_datatimestamp functional)
test_create_for_two_actors(func_limit                  functional)
test_create_for_two_actors(func_metrics                functional)
test_create_for_two_actors(func_error_handling         functional)

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
test_create_for_one_actor(unit_timestamp               unitary)

# Handle test reports
set(TEST_REPORT ${CMAKE_BINARY_DIR}/doc/tests_report.txt)

if(NOT EXISTS ${TEST_REPORT})
  file(WRITE ${TEST_REPORT} "No tests has been ran.\n")
endif()

install(FILES ${TEST_REPORT} DESTINATION ${PLATFORM_INSTALL_SUBDIR}doc)

# run_tests target
add_custom_target(run_tests ${CMAKE_CTEST_COMMAND} -O ${TEST_REPORT} --output-on-failure DEPENDS tests)
