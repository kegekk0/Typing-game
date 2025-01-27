cmake_minimum_required(VERSION 3.24)
project(pjc)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

FetchContent_Declare(
        SFML
        GIT_REPOSITORY  https://github.com/SFML/SFML
        GIT_TAG         2.6.1
)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG        10.2.1
)
FetchContent_MakeAvailable(fmt)

add_executable(pjc
        main.cpp
)

target_link_libraries(pjc fmt sfml-graphics)