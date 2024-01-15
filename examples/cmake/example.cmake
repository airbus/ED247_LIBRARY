# Add an example
# - Create target {example_name}
# - On Windows, copy ED247 library to make executables working
function (add_example_target example_name)
  add_custom_target(${example_name} ALL)

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(
      OUTPUT ${LIBED247_LIBRARY}
      COMMAND ${CMAKE_COMMAND} -E copy ${LIBED247_LIBRARIES} ${LIBED247_LIBRARY})
    add_custom_target(${example_name}_copy_libed247 DEPENDS ${LIBED247_LIBRARY})
    add_dependencies(${example_name} ${example_name}_copy_libed247)
  endif()
endfunction()


# Add a binary to an example
# - Create target {example_name}_{bin_name} that build {bin_name}
# - make {example_name} depend on {example_name}_{bin_name}
function(add_example_bin_target example_name bin_name)
  set(target_name ${example_name}_${bin_name})
  file(GLOB bin_srcs ${bin_name}/*.cpp)

  add_executable(            ${target_name} ${bin_srcs})
  set_target_properties(     ${target_name} PROPERTIES OUTPUT_NAME ${bin_name})
  target_compile_definitions(${target_name} PRIVATE "EC_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/${bin_name}/\"")
  target_include_directories(${target_name} PUBLIC ${LIBED247_INCLUDE_DIR})
  target_link_libraries(     ${target_name} PUBLIC ${LIBED247_LIBRARIES} utils)

  add_dependencies(${example_name} ${target_name})
endfunction()
