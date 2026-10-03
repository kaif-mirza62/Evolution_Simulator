cmake_minimum_required(VERSION 3.24)

project(EvolutionSimulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
)

FetchContent_MakeAvailable(raylib)

add_executable(Evolution_Simulator
    src/main.cpp
    src/environment.cpp
    src/simulation.cpp
)

target_include_directories(Evolution_Simulator PRIVATE include)

target_link_libraries(Evolution_Simulator PRIVATE raylib)