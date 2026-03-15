/**
 * \file main.cpp
 * \author Anastasiya Dorohina
 * \brief Главная логика C++23 ranges::stable_sort + проекторы
 * \date 2026-03-15
 * \version 2.1
 */

#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "median_calculator.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <boost/program_options.hpp>

namespace fs = std::filesystem;
namespace po = boost::program_options;

/**
 * \brief Главная функция: **полный ТЗ пайплайн C++23**
 * 
 * **Пайплайн обработки:**
 * 1. Boost.ProgramOptions: -config/-cfg → config.toml
 * 2. TOML++ парсинг → config с ТЗ дефолтами  
 * 3. CSV сборка → filename_mask + лексикографическая сортировка
 * 4. **ranges::stable_sort** + &MarketEvent::receive_ts проектор
 * 5. MedianCalculator O(log N) инкрементальная медиана
 * 6. **Запись только при изменении** >= 1e-8 (8 знаков)
 * 
 * **C++23 особенности:**
 * - ranges::stable_sort(events, {}, &MarketEvent::receive_ts)
 * - const auto median_opt = calc.median() pattern matching  
 * - std::setprecision(8) → ТЗ точность
 * 
 * \param[in] argc Количество аргументов CLI
 * \param[in] argv Массив аргументов CLI
 * \return 0(OK), 1(error), 255(critical)
 */
int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::info("csv_median_calculator v2.1 C++23");

    try {
        po::options_description desc("Options");
        desc.add_options()
            ("config", po::value<std::string>(), "Path to config.toml")
            ("cfg",    po::value<std::string>(), "Path to config.toml (alias)");

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
            .options(desc)
            .style(po::command_line_style::allow_long
                 | po::command_line_style::long_allow_adjacent
                 | po::command_line_style::allow_dash_for_short)
            .run(), vm);
        po::notify(vm);

        fs::path config_path;
        if (vm.count("config")) {
            config_path = vm["config"].as<std::string>();
        } else if (vm.count("cfg")) {
            config_path = vm["cfg"].as<std::string>();
        } else {
            config_path = fs::path(argv[0]).parent_path() / "config.toml";
        }

        spdlog::info("Using config: {}", config_path.string());
        
        // TOML парсинг с ТЗ дефолтами
        auto config = csv_median::parse_config(config_path);

        // CSV пайплайн: файлы → события → stable_sort
        auto events = csv_median::read_csv_files(config.input_dir, config.filename_mask);
        spdlog::info("Total events read: {}", events.size());

        if (events.empty()) {
            spdlog::warn("No events found, output file will contain only header");
            return 0;
        }

        // C++23: ranges::stable_sort + проектор
        std::ranges::stable_sort(events, {}, &csv_median::MarketEvent::receive_ts);
        spdlog::info("Sorted {} events by receive_ts", events.size());

        // ТЗ: median_result.csv перезапись
        auto output_path = config.output_dir / "median_result.csv";
        std::ofstream out(output_path);
        if (!out.is_open()) {
            spdlog::error("Cannot create output file: {}", output_path.string());
            return 1;
        }

        // ТЗ: формат + std::setprecision(8)
        out << "receive_ts;price_median\n";
        out << std::fixed << std::setprecision(8);

        csv_median::MedianCalculator calc;
        int change_count = 0;

        // C++23: const auto pattern matching
        for (const auto& event : events) {
            calc.add_price(event.price);
            if (const auto median_opt = calc.median()) {
                out << event.receive_ts << ";" << *median_opt << "\n";
                ++change_count;
            }
        }

        spdlog::info("Saved {} median changes to {}", change_count, output_path.string());
        spdlog::info("Done!");

    } catch (const po::error& ex) {
        spdlog::error("CLI error: {}", ex.what());
        return 1;
    } catch (const std::exception& ex) {
        spdlog::error("Exception: {}", ex.what());
        return 1;
    } catch (...) {
        spdlog::critical("Unknown error");
        return 255;
    }

    return 0;
}
