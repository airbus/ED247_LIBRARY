include(../cmake/example.cmake)

set(example_name example2)

add_example_target(${example_name})

add_example_bin_target(${example_name} sender)
add_example_bin_target(${example_name} receiver)
