include(${CMAKE_CURRENT_LIST_DIR}/qnx.cmake)

# Print all debug messages and use SLRT tools to filter
set(ED247_VERBOSE_DEBUG ON)

# Initialize compilation flags
set(CMAKE_CXX_FLAGS_INIT "-DSIMULINK_REAL_TIME")

# Add MATLAB / SLRT target include path to list of EXTRA_INCLUDES
set(EXTRA_INCLUDES C:/PROGRA~1/MATLAB/R2020b/toolbox/slrealtime/target/kernel/dist/include)