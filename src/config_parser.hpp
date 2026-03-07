#pragma once
#include <filesystem>
#include <vector>
#include <string>

namespace csv_median {
    struct config_t {
        std::filesystem::path input_dir_;
        std::filesystem::path output_dir_;
        std::vector<std::string> filename_mask_;
    };

    config_t parse_config(const std::filesystem::path& config_path_);
}
