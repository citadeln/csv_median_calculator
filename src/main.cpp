/**
 * \file main.cpp
 * \author Anastasiya Dorohina
 * \brief Главная логика: конфигурирование → чтение CSV → вычисление медианы → запись результата
 * \date 2026-03-08
 * \version 2.0
 */

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

/**
 * \brief Главная функция: выполняет весь рабочий процесс
 * 
 * Рабочий процесс:
 * 1. **Парсинг CLI аргументов** (Boost Program Options)
 * 2. **Чтение конфигурационного файла TOML** (ConfigParser)
 * 3. **Чтение CSV файлов** (CsvReader)
 * 4. **Стабильная сортировка данных** (Stable Sort)
 * 5. **Рассчёт медианы цен** (MedianCalculator)
 * 6. **Запись результатов в CSV** (OutputWriter)
 * 
 * Формат выходных данных: `receive_ts;price_median` (8 знаков после запятой)
 * 
 * \param[in] argc Аргументы командной строки
 * \param[in] argv Массив аргументов командной строки
 * \return 0 — успешное завершение, 1 — ошибка, 255 — критическая ошибка
 */
int main(int argc, char* argv[]) {
    /// Настройка логирования
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::info("csv_median_calculator v2.0 C++23");

    try {
        /// 1. Парсинг CLI аргументов
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

        /// 2. Чтение конфигурационного файла TOML
        fs::path config_path = vm.count("config") ? 
            fs::path(vm["config"].as<std::string>()) : "config.toml";
            
        auto config = csv_median::parse_config(config_path);
        spdlog::info("Config: input={} mask=[{}]", 
                     config.input_dir.string(),
                     config.filename_mask.empty() ? "all" :
                     (config.filename_mask.size() > 1 ? 
                      config.filename_mask[0] + ", " + config.filename_mask[1] : 
                      config.filename_mask[0]));

        /// 3. Чтение CSV файлов
        auto events = csv_median::read_csv_files(config.input_dir, config.filename_mask);
        spdlog::info("Found {} events", events.size());

        /// 4. Стабильная сортировка по временной метке
        std::stable_sort(events.begin(), events.end(), 
            [](const csv_median::MarketEvent& a, const csv_median::MarketEvent& b) {
                return a.receive_ts < b.receive_ts;
            });
        
        spdlog::info("Sorted by receive_ts: {} events", events.size());

        /// 5. Вычисление медианы цен
        csv_median::MedianCalculator calc;
        auto output_path = config.output_dir / "median_result.csv";
        
        std::ofstream out(output_path);
        if (!out.is_open()) {
            spdlog::error("❌ Cannot create {}", output_path.string());
            return 1;
        }
        
        /// 6. Запись медианы в CSV файл
        out << "receive_ts;price_median\n";
        out.precision(8);
        out << std::fixed;

        int change_count = 0;
        for (const auto& event : events) {
            calc.add_price(event.price);
            if (auto median = calc.median()) {
                out << event.receive_ts << ";" << *median << "\n";
                change_count++;
            }
        }
        out.close();

        spdlog::info("Saved {} median changes to {}", change_count, output_path.string());
        spdlog::info("✅ Done!");

    } catch (const std::exception& ex) {
        spdlog::error("Exception: {}", ex.what());
        return 1;
    } catch (...) {
        spdlog::critical("❌ Unknown error");
        return 255;
    }
    
    return 0;
}
