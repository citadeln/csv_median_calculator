#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "median_calculator.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <boost/program_options.hpp>
#include <algorithm>

namespace fs = std::filesystem;
namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::info("🚀 csv_median_calculator v2.0 C++23");

    try {
        // CLI аргументы ТЗ: ./app --config config.toml
        po::options_description desc("Options");
        desc.add_options()
            ("config,c", po::value<std::string>(), "config.toml path")
            ("help,h", "show help");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }

        // config.toml или дефолт
        fs::path config_path = vm.count("config") ? 
            fs::path(vm["config"].as<std::string>()) : "config.toml";
            
        auto config = csv_median::parse_config(config_path);
        spdlog::info("📁 Config: input={} mask=[{}]", 
                     config.input_dir.string(),
                     fmt::join(config.filename_mask, ", "));

        // Читаем CSV файлы по filename_mask
        auto events = csv_median::read_csv_files(config.input_dir, config.filename_mask);
        spdlog::info("📊 Found {} events", events.size());

        // ТЗ: std::stable_sort по receive_ts
        std::stable_sort(events.begin(), events.end(), 
            [](const csv_median::MarketEvent& a, const csv_median::MarketEvent& b) {
                return a.receive_ts < b.receive_ts;
            });
        
        spdlog::info("🔄 Sorted by receive_ts: {} events", events.size());

        // Инкрементальная медиана O(log N) ТЗ
        csv_median::MedianCalculator calc;
        auto output_path = config.output_dir / "median_result.csv";
        
        std::ofstream out(output_path);
        if (!out.is_open()) {
            spdlog::error("❌ Cannot create {}", output_path.string());
            return 1;
        }
        
        // ТЗ: формат CSV с заголовком + 8 decimals + semicolon
        out << "receive_ts;price_median\n";
        out.precision(8);
        out << std::fixed;

        // ТЗ: пересчёт медианы ПОСЛЕ КАЖДОГО price
        int change_count = 0;
        for (const auto& event : events) {
            calc.add_price(event.price);
            if (auto median = calc.median()) {
                out << event.receive_ts << ";" << *median << "\n";
                change_count++;
            }
        }
        out.close();

        spdlog::info("💾 Saved {} median changes to {}", change_count, output_path.string());
        spdlog::info("✅ Done!");

    } catch (const std::exception& ex) {
        spdlog::error("💥 Exception: {}", ex.what());
        return 1;
    } catch (...) {
        spdlog::critical("💀 Unknown error");
        return 255;
    }
    
    return 0;
}
