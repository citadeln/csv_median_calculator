cmake_minimum_required(VERSION 3.23)
project(csv_median_calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Release)

include(FetchContent)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

# toml++ С single_include ФИКСОМ!
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(csv_median_calculator
    src/main.cpp
    src/config_parser.cpp
    src/csv_reader.cpp
    src/median_calculator.cpp
    src/config_parser.hpp
    src/csv_reader.hpp
    src/median_calculator.hpp
    src/market_record.hpp
)

target_include_directories(csv_median_calculator PRIVATE 
    src
    ${tomlplusplus_SOURCE_DIR}/include 
)

target_link_libraries(csv_median_calculator PRIVATE
    spdlog::spdlog
    Boost::program_options
)

target_compile_options(csv_median_calculator PRIVATE
    -Wall -Wextra -Wpedantic
    -Wno-unused-parameter
)

set_target_properties(csv_median_calculator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

message(STATUS "🚀 csv_median_calculator C++23 configured!")
message(STATUS "📁 toml++ path: ${tomlplusplus_SOURCE_DIR}/include")
