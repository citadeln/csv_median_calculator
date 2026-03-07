cmake_minimum_required(VERSION 3.23)
project(csv_median_calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Boost.Program_options
set(BOOST_INCLUDE_LIBRARIES program_options)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.86.0
)
FetchContent_MakeAvailable(Boost)

# toml++
FetchContent_Declare(
  tomlplusplus
  GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

# spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

add_executable(csv_median_calculator
  src/main.cpp
  src/config_parser.hpp
  src/csv_reader.hpp
  src/median_calculator.hpp
)

target_include_directories(csv_median_calculator PRIVATE src)
target_link_libraries(csv_median_calculator PRIVATE
  Boost::program_options
  tomlplusplus::tomlplusplus
  spdlog::spdlog
  spdlog::spdlog_header_only
)

target_compile_definitions(csv_median_calculator PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)

set_target_properties(csv_median_calculator PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
