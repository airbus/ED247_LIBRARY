find_package(Doxygen)
if (NOT Doxygen_FOUND)
  message(WARNING "Doxygen not found. API Documentation will not be generated!")
  return()
endif()

# Global configuration
set(DOX_PROJECT_VERSION ${VERSION})
set(DOX_INPUT "${CMAKE_CURRENT_BINARY_DIR}/doc.dox ${PROJECT_SOURCE_DIR}/src")
set(DOX_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
configure_file(doxyfile.in doxyfile @ONLY)

# Generate content
file(READ ${PROJECT_SOURCE_DIR}/README.md DOX_README)
file(READ ${PROJECT_SOURCE_DIR}/LICENSE.md DOX_LICENSE)
file(READ ${PROJECT_SOURCE_DIR}/src/ed247/xsd/ED247A_ECD.xsd DOX_XSD_ECD)
file(READ ${PROJECT_SOURCE_DIR}/src/ed247/xsd/ED247A_ECIC.xsd DOX_XSD_ECIC)
configure_file(doc.dox.in doc.dox @ONLY)

# List dependencies
file(GLOB DOX_DEPENDS LIST_DIRECTORIES false
  ${PROJECT_SOURCE_DIR}/*.md
  ${PROJECT_SOURCE_DIR}/doc/doxygen/*
  ${PROJECT_SOURCE_DIR}/src/ed247/*.h
  ${PROJECT_SOURCE_DIR}/src/ed247/*.cpp
  ${PROJECT_SOURCE_DIR}/src/ed247/xsd/*.xsd
  )

# Command
set(DOX_CMD ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)
add_custom_command(OUTPUT doxygen/html COMMAND ${DOX_CMD} DEPENDS ${DOX_DEPENDS})

# Targets
add_custom_target(doxygen DEPENDS doxygen/html)
add_dependencies(doc doxygen)

# Install
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html DESTINATION doc)
