#include "config_parser.hpp"
#include <toml.hpp>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace csv_median {
namespace fs = std::filesystem;

Config parse_config(const fs::path& path) {
    Config config;
    
    // Дефолтные значения (ТЗ)
    config.input_dir_ = "./data";
    config.filename_mask_ = {"trade"};
    config.output_dir_ = "./output";
    
    if (!fs::exists(path)) {
        spdlog::warn("⚠️ Config not found: {}, using defaults", path.string());
        return config;
    }

    try {
        // ✅ РЕАЛЬНЫЙ TOML парсинг (ТЗ format)
        auto data = toml::parse_file(path.string());
        auto main_table = toml::find<toml::table>(data, "main");
        
        config.input_dir_ = main_table["input"].value<std::string>();
        config.output_dir_ = main_table["output"].value_or("./output");
        
        // ✅ filename_mask = ['level', 'trade'] (ТЗ)
        if (auto mask_array = main_table["filename_mask"]; mask_array.is_array()) {
            for (size_t i = 0; i < mask_array.as_array()->size(); ++i) {
                config.filename_mask_.push_back((*mask_array.as_array())[i].value<std::string>());
            }
        }
        
    } catch (const std::exception& e) {
        spdlog::warn("❌ TOML parse error: {}, using defaults", e.what());
    }
    
    // Создаём output директорию (ТЗ)
    if (!config.output_dir_.empty() && !fs::exists(config.output_dir_)) {
        fs::create_directories(config.output_dir_);
    }
    
    spdlog::info("⚙️ Config loaded: input={} mask=[{}] output={}", 
                 config.input_dir_, 
                 fmt::join(config.filename_mask_, ", "), 
                 config.output_dir_);
    
    return config;
}

} // namespace csv_median
