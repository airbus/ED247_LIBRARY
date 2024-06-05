add_executable(chatbot chatbot.cpp)

target_link_libraries(chatbot
  PRIVATE
  Ed247::ed247
  libtime_tools
)

install(TARGETS chatbot RUNTIME DESTINATION ${PLATFORM_INSTALL_SUBDIR}bin)
