cmake_minimum_required(VERSION 3.30)
include(FetchContent)
project(BinaryInterleave)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(lyra
        GIT_REPOSITORY https://github.com/bfgroup/Lyra.git
        GIT_TAG 1.6.1)

FetchContent_MakeAvailable(lyra)

add_executable(binaryInterleave main.cpp)

target_link_libraries(binaryInterleave lyra)