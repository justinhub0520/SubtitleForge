add_executable(test_server test_server.cpp)
target_link_libraries(test_server PRIVATE subforge_backend shared)
add_test(NAME test_server COMMAND test_server)