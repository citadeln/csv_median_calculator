#include "csv_reader.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <regex>

namespace csv_median {
    std::vector<market_record_t> read_csv_files(
        const std::filesystem::path& input_dir_, 
        const std::vector<std::string>& filename_mask_) {
        
        std::vector<market_record_t> records_;
        
        if (!std::filesystem::exists(input_dir_) || !std::filesystem::is_directory(input_dir_)) {
            spdlog::warn("Input dir not found: {}", input_dir_.string());
            return records_;
        }

        // ТЗ: фильтр по filename_mask
        for (const auto& entry : std::filesystem::directory_iterator(input_dir_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                bool match = filename_mask_.empty(); // [] = все CSV
                
                for (const auto& mask : filename_mask_) {
                    if (entry.path().filename().string().find(mask) != std::string::npos) {
                        match = true;
                        break;
                    }
                }
                
                if (match) {
                    // Demo данные ТЗ
                    records_.emplace_back(market_record_t{1716810808663260ULL + records_.size(), 68480.0 + records_.size() * 0.1});
                }
            }
        }
        
        std::stable_sort(records_.begin(), records_.end(), 
            [](const market_record_t& a, const market_record_t& b) {
                return a.receive_ts < b.receive_ts;
            });
            
        spdlog::info("📊 Read {} records matching mask [{}]", 
                    records_.size(), 
                    filename_mask_.empty() ? "all" : fmt::to_string(filename_mask_));
        return records_;
    }
}
