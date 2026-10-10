cmake_minimum_required(VERSION 3.24)

project(EvolutionSimulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(RAYLIB_VERSION 6.0)

FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
)

FetchContent_MakeAvailable(raylib)

add_executable(EvolutionSimulator
    src/main.cpp
    src/brain.cpp
    src/creature.cpp
    src/environment.cpp
    src/simulation.cpp
    src/neural_network.cpp
    src/replay_buffer.cpp
    src/rl_agent.cpp
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