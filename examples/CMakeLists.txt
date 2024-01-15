cmake_minimum_required(VERSION 3.22)
project(ED247_EXAMPLES VERSION 1.0.0) # Define target version ${CMAKE_PROJECT_VERSION})
include(cmake/version.cmake)          # Compute ${VERSION} according to git
include(cmake/flags.cmake)

# libED247 (Similar to find_package)
if(DEFINED ENV{LIBED247_ROOT})
  set(LIBED247_ROOT $ENV{LIBED247_ROOT})
else()
  message(WARNING "LIBED247_ROOT env variable is not set. Find package may not works.")
endif()
set(LIBED247_LIBRARY libed247${CMAKE_SHARED_LIBRARY_SUFFIX})
find_file(LIBED247_LIBRARIES ${LIBED247_LIBRARY}
  PATHS "${LIBED247_ROOT}/lib" "${LIBED247_ROOT}/lib32" "${LIBED247_ROOT}/lib64" "${LIBED247_ROOT}/bin"
  REQUIRED)
find_path(LIBED247_INCLUDE_DIR ed247.h
  PATHS "${LIBED247_ROOT}/include" "${LIBED247_ROOT}/ncl" "${LIBED247_ROOT}/inc"
  REQUIRED)


add_subdirectory(utils)

# Add all examples
file(GLOB examples example*)
foreach(example ${examples})
  add_subdirectory(${example})
endforeach()
