cmake_minimum_required(VERSION 3.30)
include(FetchContent)
project(WinCETool)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(lyra
        GIT_REPOSITORY https://github.com/bfgroup/Lyra.git
        GIT_TAG 1.6.1)

FetchContent_MakeAvailable(lyra)

FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.2)

FetchContent_MakeAvailable(spdlog)

add_executable(winCETool main.cpp)

target_link_libraries(winCETool lyra)
target_link_libraries(winCETool spdlog)
