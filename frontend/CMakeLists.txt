option(BUILD_FRONTEND "Build frontend application" ON)

if(BUILD_FRONTEND)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Network)

    add_executable(subforge_frontend
        src/main.cpp
        src/MainWindow.cpp
        src/VideoPlayerWidget.cpp
        src/TimelineEditor.cpp
        src/SubtitleListWidget.cpp
        src/ApiClient.cpp
        src/StyleEditorWidget.cpp
    )

    target_link_libraries(subforge_frontend PRIVATE
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::Network
        shared
    )

    target_include_directories(subforge_frontend PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
endif()