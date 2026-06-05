cmake_minimum_required(VERSION 3.24)

project(color_puzzle_solver
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(boost_container REQUIRED CONFIG)

add_executable(color_puzzle_solver
    src/main.cpp
    src/util.cpp
    src/color_puzzle.cpp
    src/game_state_set.cpp
)

target_compile_features(color_puzzle_solver
    PRIVATE
        cxx_std_23
)

target_link_libraries(color_puzzle_solver
    PRIVATE
        Boost::container
)

if(MSVC)
    target_compile_options(color_puzzle_solver PRIVATE /W4 /permissive-)
else()
    target_compile_options(color_puzzle_solver PRIVATE -Wall -Wextra -Wpedantic)
endif()