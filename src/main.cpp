/**
 * \file main.cpp
 * \author Anastasiya Dorohina
 * \brief StatsCalculator + multi-metrics ТЗ 7.2 (C++23) **полный пайплайн**
 * \date 2026-03-15
 * \version 2.2
 */

#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "stats_calculator.hpp"  ///< ТЗ 7.2: Boost.Accumulators
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <algorithm>             ///< C++23 ranges::stable_sort
#include <boost/program_options.hpp>

namespace fs = std::filesystem;
namespace po = boost::program_options;

/**
 * \brief Главная функция: **полный ТЗ пайплайн C++23 + ТЗ 7.2 метрики**
 * 
 * **Пайплайн обработки:**
 * 1. **Boost.ProgramOptions:** `-config`/`-cfg` → `config.toml`
 * 2. **TOML++ парсинг** → `Config` с ТЗ дефолтами  
 * 3. **CSV сборка** → `filename_mask` + лексикографическая сортировка файлов
 * 4. **C++23 `ranges::stable_sort`** + `&MarketEvent::receive_ts` проектор
 * 5. **StatsCalculator Boost.Accumulators** → mean/std/p50/p90/p95/p99/median
 * 6. **Запись только при изменении** ≥ 1e-8 для **всех метрик** (ТЗ)
 * 
 * **C++23 особенности:**
 * - `ranges::stable_sort(events, {}, &MarketEvent::receive_ts)`
 * - `std::setprecision(8)` → ТЗ точность 8 знаков
 * - `std::optional<double>` → фильтрация изменений
 * 
 * **Пример выходного файла `median_result.csv` (ТЗ 7.2):**
 * ```
 * receive_ts;median;mean;std_dev;p90;p95
 * 1716810808593627;68480.00000000;68480.05000000;0.12345678;68480.10000000;68480.20000000
 * ```
 * 
 * \param[in] argc Количество аргументов CLI
 * \param[in] argv Массив аргументов CLI  
 * \return 0(OK), 1(error), 255(critical)
 * \Complexity O(F log F + N log N) где F — файлы, N — события
 */
int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::info("csv_median_calculator v2.2 C++23 + Boost.Accumulators (ТЗ 7.2)");

    try {
        /**
         * **1. CLI парсинг Boost.ProgramOptions**
         * Поддержка: `--config`, `--cfg`, автоопределение config.toml
         */
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
        
        // **2. TOML парсинг с ТЗ дефолтами** (metrics из ТЗ 7.2)
        auto config = csv_median::parse_config(config_path);

        // **3. CSV пайплайн ТЗ 3.1** 
        auto events = csv_median::read_csv_files(config.input_dir, config.filename_mask);
        spdlog::info("Total events read: {}", events.size());

        if (events.empty()) {
            spdlog::warn("No events found, output file will contain only header");
            return 0;
        }

        // **4. C++23: ranges::stable_sort + проектор**
        std::ranges::stable_sort(events, {}, &csv_median::MarketEvent::receive_ts);
        spdlog::info("Sorted {} events by receive_ts", events.size());

        // **5. ТЗ: median_result.csv (НЕ stats_result.csv)**
        auto output_path = config.output_dir / "median_result.csv";
        std::ofstream out(output_path);
        if (!out.is_open()) {
            spdlog::error("Cannot create output: {}", output_path.string());
            return 1;
        }

        // **6. StatsCalculator ИНИЦИАЛИЗИРОВАН ДО использования (ТЗ 7.2)**
        csv_median::StatsCalculator stats_calc(config.metrics);
        const auto& headers = stats_calc.get_headers();
        
        // Запись заголовков (динамически по metrics[])
        for (size_t i = 0; i < headers.size(); ++i) {
            if (i > 0) out << ";";
            out << headers[i];
        }
        out << "\n";
        out << std::fixed << std::setprecision(8);  ///< ТЗ: 8 знаков точности

        int change_count = 0;
        std::vector<double> last_stats(headers.size() - 1, 0.0);  ///< Без receive_ts

        /**
         * **7. Основной цикл: фильтрация изменений для ВСЕХ метрик**
         * - Boost.Accumulators: инкрементальная статистика O(1)
         * - Фильтр: |current - last| ≥ 1e-8 для любой метрики → запись
         */
        for (const auto& event : events) {
            stats_calc.add_price(event.price);
            auto stats = stats_calc.get_stats();
            
            if (!stats.empty()) {
                // Проверка изменений по любой метрике (ТЗ оптимизация)
                bool has_changes = false;
                for (size_t i = 0; i < stats.size(); ++i) {
                    if (stats[i] && std::fabs(*stats[i] - last_stats[i]) >= 1e-8) {
                        has_changes = true;
                        break;
                    }
                }
                
                if (has_changes) {
                    // Запись строки: receive_ts + все метрики
                    out << event.receive_ts;
                    for (const auto& stat : stats) {
                        out << ";" << (stat ? *stat : 0.0);
                    }
                    out << "\n";
                    
                    // Обновление last_stats для следующей итерации
                    for (size_t i = 0; i < stats.size(); ++i) {
                        last_stats[i] = stat ? *stat : 0.0;
                    }
                    ++change_count;
                }
            }
        }

        spdlog::info("Saved {} rows with {} metrics to {}", 
            change_count, config.metrics.size(), output_path.string());
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
