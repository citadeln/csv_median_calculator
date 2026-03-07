cmake_minimum_required(VERSION 3.23)
project(csv_median_calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

include(FetchContent)

# spdlog (zero-install)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

# toml++ (zero-install)  
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

# Boost (optional)
find_package(Boost QUIET COMPONENTS program_options)
if(Boost_FOUND)
    message(STATUS "✅ Boost found: ${Boost_VERSION}")
endif()

add_executable(csv_median_calculator
    src/main.cpp
    src/config_parser.hpp  
    src/csv_reader.hpp
    src/median_calculator.hpp
)

target_link_libraries(csv_median_calculator PRIVATE
    spdlog::spdlog
    tomlplusplus::tomlplusplus
)

if(Boost_FOUND)
    target_link_libraries(csv_median_calculator PRIVATE Boost::program_options)
    target_compile_definitions(csv_median_calculator PRIVATE HAS_BOOST_CLI)
endif()

target_compile_options(csv_median_calculator PRIVATE 
    -Wall -Wextra -Wpedantic -Werror
)
