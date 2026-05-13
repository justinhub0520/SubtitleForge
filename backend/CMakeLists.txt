option(BUILD_BACKEND "Build backend server" ON)

if(BUILD_BACKEND)
    add_executable(subforge_backend
        src/main.cpp
        src/server.cpp
        src/config.cpp
        src/audio_extractor.cpp
        src/transcriber.cpp
        src/task_manager.cpp
        src/srt_parser.cpp
        src/upload_handler.cpp
        src/transcribe_handler.cpp
        src/export_handler.cpp
    )

    target_link_libraries(subforge_backend PRIVATE shared)

    target_include_directories(subforge_backend PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    find_package(OpenSSL REQUIRED)
    find_package(ZLIB REQUIRED)

    target_link_libraries(subforge_backend PRIVATE OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB pthread)

    enable_testing()
    add_subdirectory(tests)
endif()