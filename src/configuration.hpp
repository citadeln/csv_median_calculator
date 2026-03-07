#pragma once

#include <string>
#include <filesystem>

namespace MarketDataProcessor {
namespace Config {

struct Configuration {
    std::filesystem::path input_dir;
    std::filesystem::path output_file;
    std::string timestamp_column_name = "timestamp";
    std::string price_column_name = "price";
};

} // namespace Config
} // namespace MarketDataProcessor
