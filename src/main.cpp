/**
 * \file main.cpp
 * \author Anastasiya Dorohina
 * \brief Главная логика: конфигурирование -> чтение CSV -> вычисление медианы -> запись результата
 * \date 2026-03-08
 * \version 2.0
 */

#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "median_calculator.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <boost/program_options.hpp>

namespace fs = std::filesystem;
namespace po = boost::program_options;

/**
 * \brief Главная функция: выполняет весь рабочий процесс **строго по ТЗ**
 * 
 * **Полный пайплайн:**
 * 1. **Парсинг CLI** (Boost.Program_options): -config/-cfg или config.toml рядом с бинарником
 * 2. **TOML парсинг** (toml++): валидация + ТЗ дефолты при ошибке  
 * 3. **CSV сборка**: recursive scan + filename_mask + лексикографическая сортировка файлов
 * 4. **stable_sort** по receive_ts (сохраняет порядок файлов при равных ts)
 * 5. **Инкрементальная медиана** O(log N) через MedianCalculator
 * 6. **Запись только при изменении** медианы >= 1e-8 (8 знаков точности)
 * 7. **median_result.csv** в output_dir (перезапись при повторном запуске)
 * 
 * **Обработка ошибок:** 
 * - CLI → po::error (return 1)
 * - TOML/файлы → ТЗ дефолты или пустой результат  
 * - Файловая система → spdlog + graceful выход
 * - **Никогда не crash** (catch(...))
 * 
 * \param[in] argc Количество аргументов командной строки
 * \param[in] argv Массив аргументов командной строки  
 * \return 0 (OK), 1 (ошибка), 255 (критическая)
 */
int main(int argc, char* argv[]) {
    // ТЗ: spdlog паттерн + версия приложения
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::info("csv_median_calculator v2.0 C++23");

    try {
        // 1. Boost.Program_options: -config / -cfg
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

        // Определение пути config.toml (ТЗ: дефолт рядом с exe)
        fs::path config_path;
        if (vm.count("config")) {
            config_path = vm["config"].as<std::string>();
        } else if (vm.count("cfg")) {
            config_path = vm["cfg"].as<std::string>();
        } else {
            config_path = fs::path(argv[0]).parent_path() / "config.toml";
        }

        spdlog::info("Using config: {}", config_path.string());

        // 2. TOML парсинг + валидация (ТЗ дефолты при ошибке)
        auto config = csv_median::parse_config(config_path);

        // 3. CSV сборка: все файлы по маске → лексикографический порядок
        auto events = csv_median::read_csv_files(config.input_dir, config.filename_mask);
        spdlog::info("Total events read: {}", events.size());

        if (events.empty()) {
            spdlog::warn("No events found, output file will contain only header");
        }

        // 4. ТЗ: stable_sort сохраняет порядок файлов при равных receive_ts
        std::ranges::stable_sort(events, {}, &csv_median::MarketEvent::receive_ts);
        spdlog::info("Sorted {} events by receive_ts", events.size());

        // 5. Выходной файл median_result.csv (ТЗ: перезапись)
        auto output_path = config.output_dir / "median_result.csv";
        std::ofstream out(output_path);
        if (!out.is_open()) {
            spdlog::error("Cannot create output file: {}", output_path.string());
            return 1;
        }

        // ТЗ: формат + 8 знаков точности
        out << "receive_ts;price_median\n";
        out << std::fixed << std::setprecision(8);

        // 6. Инкрементальная медиана O(log N)
        csv_median::MedianCalculator calc;
        int change_count = 0;

        // ТЗ: запись ТОЛЬКО при изменении медианы >= 1e-8
        for (const auto& event : events) {
            calc.add_price(event.price);
            if (const auto median_opt = calc.median()) {
                out << event.receive_ts << ";" << *median_opt << "\n";
                ++change_count;
            }
        }

        out.close();
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
