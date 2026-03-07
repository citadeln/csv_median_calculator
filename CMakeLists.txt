cmake_minimum_required(VERSION 3.23)
project(csv_median_calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

# toml++
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(csv_median_calculator
    src/main.cpp
    src/config_parser.cpp
    src/csv_reader.cpp
    src/median_calculator.cpp
)

target_link_libraries(csv_median_calculator PRIVATE
    spdlog::spdlog
    tomlplusplus::tomlplusplus
    Boost::program_options
)

target_compile_options(csv_median_calculator PRIVATE -Wall -Wextra -Wpedantic)
