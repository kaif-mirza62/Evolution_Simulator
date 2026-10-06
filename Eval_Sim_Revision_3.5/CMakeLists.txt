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

add_executable(EvolutionSimulator
    src/main.cpp
    src/brain.cpp
    src/creature.cpp
    src/environment.cpp
    src/simulation.cpp
)

target_include_directories(
    EvolutionSimulator
    PRIVATE
    include
)

target_link_libraries(
    EvolutionSimulator
    PRIVATE
    raylib
)
