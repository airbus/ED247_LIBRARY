add_executable(chatbot chatbot.cpp)
install(TARGETS chatbot RUNTIME DESTINATION ${PLATFORM_INSTALL_SUBDIR}bin)
