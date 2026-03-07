#pragma once
#include "market_record.hpp"
#include <filesystem>
#include <vector>
#include <string>

namespace csv_median {
    std::vector<market_record_t> read_csv_files(
        const std::filesystem::path& input_dir_,
        const std::vector<std::string>& filename_mask_);
}
