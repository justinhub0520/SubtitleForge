add_executable(test_server test_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/task_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/srt_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/upload_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/transcribe_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/export_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/audio_extractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/transcriber.cpp
)
target_link_libraries(test_server PRIVATE shared OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB pthread)
target_include_directories(test_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
add_test(NAME test_server COMMAND test_server)