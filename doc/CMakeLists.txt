add_custom_target(doc ALL)
add_subdirectory(doxygen)
include(delivery_note.cmake OPTIONAL) # Airbus product delivery note
