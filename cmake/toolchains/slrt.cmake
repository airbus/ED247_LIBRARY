include(${CMAKE_CURRENT_LIST_DIR}/qnx.cmake)

# Initialize compilation flags
set(CMAKE_CXX_FLAGS_INIT "-DSIMULINK_REAL_TIME")

# Add MATLAB / SLRT target include path to list of EXTRA_INCLUDES
set(EXTRA_INCLUDES C:/PROGRA~1/MATLAB/R2020b/toolbox/slrealtime/target/kernel/dist/include)