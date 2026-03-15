/**
 * \file config_parser.cpp
 * \author Anastasiya Dorohina
 * \brief TOML++ v3.4.0 парсер конфигурации
 * \date 2026-03-08
 * \version 2.0
 */

#include "config_parser.hpp"
#include <toml++/toml.hpp>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace csv_median {
namespace fs = std::filesystem;

Config parse_config(const fs::path& path) {
    Config config;

    // Инициализация ТЗ дефолтами
    config.input_dir = "./data";
    config.filename_mask = {"level", "trade"};
    config.output_dir = "./output";

    // Файл не существует — ТЗ дефолты
    if (!fs::exists(path)) {
        spdlog::warn("⚠️ Config {} not found, using defaults", path.string());
        fs::create_directories(config.output_dir);
        return config;
    }

    try {
        // Парсинг TOML файла
        auto data = toml::parse_file(path.string());

        // Доступ к секции [main]
        if (auto main_table = data["main"]; main_table && main_table.is_table()) {
            // Преобразование [main].input: string → fs::path
            if (auto input_val = main_table["input"]; input_val && input_val.is_string()) {
                config.input_dir = fs::path(input_val.as_string()->get());
            }

            // Преобразование [main].output: string → fs::path
            if (auto output_val = main_table["output"]; output_val && output_val.is_string()) {
                config.output_dir = fs::path(output_val.as_string()->get());
            }

            // [main].filename_mask: array → vector<string>
            if (auto mask_array = main_table["filename_mask"]; mask_array && mask_array.is_array()) {
                config.filename_mask.clear();
                // Итерация по *as_array() (TOML++ v3.4.0)
                for (const auto& item : *mask_array.as_array()) {
                    if (item.is_string()) {
                        config.filename_mask.push_back(item.as_string()->get());
                    }
                }
            }

            // [main].metrics: array → vector<string> (ТЗ 7.2)
            if (auto metrics_array = main_table["metrics"]; metrics_array && metrics_array.is_array()) {
                config.metrics.clear();
                for (const auto& item : *metrics_array.as_array()) {
                    if (item.is_string()) {
                        config.metrics.push_back(item.as_string()->get());
        }
    }
    
    spdlog::info("Metrics: {}", 
    config.metrics.empty() ? "none" : 
    (config.metrics.size() == 1 ? config.metrics[0] : 
     config.metrics[0] + ", " + config.metrics[1]));

}

            // Логирование результата парсинга
            spdlog::info("✅ TOML parsed: input='{}' mask=[{}]",
                        config.input_dir.string(),
                        config.filename_mask.empty() ? "none" :
                        (config.filename_mask.size() > 1 ?
                        config.filename_mask[0] + ", " + config.filename_mask[1] :
                        config.filename_mask[0]));
        }
    } catch (const std::exception& e) {
        // Любая ошибка TOML — ТЗ дефолты
        spdlog::warn("❌ TOML error '{}', using defaults", e.what());
    }

    // Гарантируем создание output_dir
    fs::create_directories(config.output_dir);
    return config;
}

} // namespace csv_median
