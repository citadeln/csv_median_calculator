#pragma once
#include "market_record.hpp"
#include <filesystem>
#include <vector>
#include <string>

namespace csv_median {
namespace fs = std::filesystem;

std::vector<MarketEvent> parse_csv(const fs::path& file);
std::vector<MarketEvent> read_csv_files(const fs::path& input_dir, const std::vector<std::string>& filename_mask);

} // namespace csv_median
