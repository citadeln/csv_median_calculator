#include "config_parser.hpp"
#include <toml++/toml.hpp>  // ✅ Правильный путь!
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fmt/ranges.h>

namespace csv_median {
namespace fs = std::filesystem;

Config parse_config(const fs::path& path) {
    Config config;
    
    // Дефолтные значения ТЗ
    config.input_dir = "./data";
    config.filename_mask = {"trade"};
    config.output_dir = "./output";
    
    if (!fs::exists(path)) {
        spdlog::warn("⚠️ Config {} not found, using defaults", path.string());
        return config;
    }

    try {
        // ✅ РЕАЛЬНЫЙ TOML ПАРСИНГ ТЗ!
        auto data = toml::parse_file(path.string());
        auto main_table = toml::find<toml::table>(data, "main");
        
        // ТЗ: [main].input (обязательный)
        config.input_dir = main_table.at("input").value_or(config.input_dir).value<std::string>();
        
        // ТЗ: [main].output (опциональный)
        if (auto output_val = main_table["output"]) {
            config.output_dir = output_val.value<std::string>();
        }
        
        // ТЗ: [main].filename_mask = ['level', 'trade']
        if (auto mask_array = main_table["filename_mask"]; mask_array && mask_array.is_array()) {
            config.filename_mask.clear();
            for (size_t i = 0; i < mask_array.as_array()->size(); ++i) {
                config.filename_mask.push_back(
                    (*mask_array.as_array())[i].value_or<std::string>("")
                );
            }
        }
        
        spdlog::info("✅ TOML parsed: input={} mask=[{}] output={}", 
                     config.input_dir.string(),
                     fmt::join(config.filename_mask, ", "), 
                     config.output_dir.string());
                     
    } catch (const std::exception& e) {
        spdlog::warn("❌ TOML error '{}', using defaults", e.what());
    }
    
    // ТЗ: создать output_dir если нет
    fs::create_directories(config.output_dir);
    return config;
}

} // namespace csv_median
