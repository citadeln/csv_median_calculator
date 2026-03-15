/**
 * \file main.cpp
 * \author Anastasiya Dorohina
 * \brief ТЗ 7.2: запись при изменении **МЕДИАНЫ**, вывод **ВСЕХ метрик**
 */

#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "stats_calculator.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <boost/program_options.hpp>

namespace fs = std::filesystem;
namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::info("csv_median_calculator v2.2 ТЗ 7.2 multi-metrics");

    try {
        // CLI парсинг
        po::options_description desc("Options");
        desc.add_options()
            ("config", po::value<std::string>(), "Path to config.toml")
            ("cfg", po::value<std::string>(), "Path to config.toml (alias)");

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
            .options(desc)
            .run(), vm);
        po::notify(vm);

        fs::path config_path = vm.count("config") ? fs::path(vm["config"].as<std::string>())
            : vm.count("cfg") ? fs::path(vm["cfg"].as<std::string>())
            : fs::path(argv[0]).parent_path() / "config.toml";

        spdlog::info("Using config: {}", config_path.string());
        auto config = csv_median::parse_config(config_path);

        // CSV пайплайн ТЗ 3.1
        auto events = csv_median::read_csv_files(config.input_dir, config.filename_mask);
        spdlog::info("Total events read: {}", events.size());

        if (events.empty()) {
            spdlog::warn("No events found");
            return 0;
        }

        // C++23 stable_sort по receive_ts
        std::ranges::stable_sort(events, {}, &csv_median::MarketEvent::receive_ts);
        spdlog::info("Sorted {} events", events.size());

        auto output_path = config.output_dir / "median_result.csv";
        std::ofstream out(output_path);
        if (!out) {
            spdlog::error("Cannot create: {}", output_path.string());
            return 1;
        }

        // StatsCalculator ТЗ 7.2
        csv_median::StatsCalculator stats_calc(config.metrics);
        auto headers = stats_calc.get_headers();

        // Заголовки
        for (size_t i = 0; i < headers.size(); ++i) {
            if (i > 0) out << ";";
            out << headers[i];
        }
        out << "\n";
        out << std::fixed << std::setprecision(8);

        double last_median = 0.0; 
        int change_count = 0;

        for (const auto& event : events) {
            stats_calc.add_price(event.price);
            auto stats = stats_calc.get_stats();
            
            if (!stats.empty()) {
                // Проверяем ТОЛЬКО медиану (индекс 0)
                auto median_opt = stats[0];  
                if (median_opt && std::fabs(*median_opt - last_median) >= 1e-8) {
                    // Записываем ВСЕ метрики
                    out << event.receive_ts;
                    for (const auto& stat : stats) {
                        out << ";" << (stat ? *stat : 0.0);
                    }
                    out << "\n";
                    
                    last_median = *median_opt;
                    ++change_count;
                }
            }
        }

        spdlog::info("Saved {} rows (median changes) with {} metrics to {}", 
            change_count, config.metrics.size(), output_path.string());
        spdlog::info("Done!");

    } catch (const po::error& ex) {
        spdlog::error("CLI error: {}", ex.what());
        return 1;
    } catch (const std::exception& ex) {
        spdlog::error("Error: {}", ex.what());
        return 1;
    } catch (...) {
        spdlog::critical("Unknown error");
        return 255;
    }

    return 0;
}
