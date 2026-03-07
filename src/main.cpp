/// \file main.cpp
/// \brief Entry point: CLI → Config → CSV processing → Hybrid median → median_result.csv
/// \author Anastasia Dorohina
/// \version 2.0
/// \date 2026-03-07
#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "median_calculator.hpp"
#include "file_discovery.hpp"

#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

/// \brief Event structure: receive_ts + price для stable_sort
struct event_t {
    uint64_t receive_ts;
    double price;
    
    /// Stable_sort comparator
    bool operator<(const event_t& other_) const noexcept {
        return receive_ts < other_.receive_ts;
    }
};

int main(int argc_, char* argv_[]) {
    /// spdlog setup: stdout + pattern
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::info("🚀 Starting csv_median_calculator v2.0 (C++23)");

    try {
        /// 1. CLI + Config parsing (Boost optional)
        auto config_ = cfg::parse_config(argc_, argv_[]);
        spdlog::info("📁 Input: {}, Output: {}", 
                     config_.input_dir_.string(), config_.output_dir_.string());

        /// 2. Create output directory
        fs::create_directories(config_.output_dir_);
        auto output_path_ = config_.output_dir_ / "median_result.csv";

        /// 3. File discovery + filename_mask filter
        auto csv_files_ = file_discovery::discover_csv_files(config_.input_dir_, config_.filename_mask_);
        spdlog::info("📋 Found {} CSV files matching mask {:?}", csv_files_.size(), 
                     config_.filename_mask_);

        if (csv_files_.empty()) {
            spdlog::warn("⚠️  No matching CSV files found — exiting");
            return 0;
        }

        /// 4. Read ALL events (200MB optimized)
        std::vector<event_t> events_;
        events_.reserve(1024 * 1024);  /// Pre-allocate для 1M+ events (200MB opt)

        for (const auto& file_info_ : csv_files_) {
            spdlog::debug("📄 Processing: {} ({})", 
                         file_info_.path.filename().string(), file_info_.format);
            
            csv::reader reader_(file_info_.path, file_info_.format);
            while (auto record_opt_ = reader_.next()) {
                const auto& record_ = *record_opt_;
                events_.emplace_back(event_t{record_.receive_ts, static_cast<double>(record_.price)});
            }
        }

        spdlog::info("📊 Collected {} price events", events_.size());

        /// 5. ТЗ: std::stable_sort по receive_ts (collisions stable!)
        std::stable_sort(events_.begin(), events_.end());
        spdlog::info("✅ Sorted {} events (stable_sort receive_ts)", events_.size());

        /// 6. Hybrid median calculator (nth_element + P²)
        median::calculator median_calc_;
        
        /// Output file: fixed "median_result.csv" (overwrite!)
        std::ofstream output_stream_(output_path_.string());
        if (!output_stream_.is_open()) {
            spdlog::error("❌ Cannot create output: {}", output_path_.string());
            return 1;
        }
        
        /// CSV header
        output_stream_ << "receive_ts;price_median\n";
        output_stream_.setf(std::ios::fixed);
        output_stream_.precision(8);

        /// 7. Incremental median calculation
        std::optional<double> prev_median_;
        std::size_t changes_written_ = 0;

        for (const auto& event_ : events_) {
            median_calc_.add_price(event_.price);
            
            auto current_median_opt_ = median_calc_.median();
            if (!current_median_opt_) continue;

            const double current_median_ = *current_median_opt_;
            
            /// Записываем ТОЛЬКО при изменении (8 decimals)
            if (!prev_median_ || std::abs(current_median_ - *prev_median_) > 1e-8) {
                output_stream_ << event_.receive_ts << ';' << current_median_ << '\n';
                prev_median_ = current_median_;
                ++changes_written_;
            }
        }

        output_stream_.close();
        spdlog::info("✨ Completed: {} events processed, {} median changes written", 
                     events_.size(), changes_written_);
        spdlog::info("💾 Result: {}", output_path_.string());

    } catch (const std::filesystem::filesystem_error& fs_err_) {
        spdlog::error("💥 Filesystem error: {}", fs_err_.what());
        return 2;
    } catch (const cfg::config_error& cfg_err_) {
        spdlog::error("⚙️  Config error: {}", cfg_err_.what());
        return 3;
    } catch (const csv::parse_error& csv_err_) {
        spdlog::error("📄 CSV parse error: {}", csv_err_.what());
        return 4;
    } catch (const std::exception& ex_) {
        spdlog::critical("💀 Fatal error: {}", ex_.what());
        return 1;
    } catch (...) {
        spdlog::critical("💀 Unknown exception");
        return 255;
    }

    spdlog::info("✅ csv_median_calculator finished successfully");
    return 0;
}
