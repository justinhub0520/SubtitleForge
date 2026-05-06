cmake_minimum_required(VERSION 3.16)
project(SubForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_BACKEND "Build backend server" ON)
option(BUILD_FRONTEND "Build frontend application" ON)

add_subdirectory(shared)

if(BUILD_BACKEND)
    add_subdirectory(backend)
endif()

if(BUILD_FRONTEND)
    add_subdirectory(frontend)
endif()