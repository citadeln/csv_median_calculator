#pragma once
#include <filesystem>
#include <vector>
#include <string>

namespace csv_median {
namespace fs = std::filesystem;

struct Config {
    fs::path input_dir;
    fs::path output_dir;
    std::vector<std::string> filename_mask;
};

Config parse_config(const fs::path& path);

} // namespace csv_median
