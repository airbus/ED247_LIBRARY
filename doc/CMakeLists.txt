add_custom_target(doc ALL)
add_subdirectory(doxygen)

# Airbus product documentation
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/airbus)
  add_subdirectory(airbus)
endif()
