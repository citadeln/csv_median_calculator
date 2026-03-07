#include "config_parser.hpp"
#include <toml.hpp>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace csv_median {
    config_t parse_config(const std::filesystem::path& config_path_) {
        config_t config;
        
        if (!std::filesystem::exists(config_path_)) {
            spdlog::warn("Config not found: {}, using defaults", config_path_.string());
            config.input_dir_ = "./data";
            config.output_dir_ = "./results";
            config.filename_mask_ = {"trade"};
            return config;
        }

        try {
            auto data = toml::parse(config_path_);
            auto main_table = toml::find<toml::table>(data, "main");
            
            config.input_dir_ = toml::find_or(main_table, "input", "./data");
            config.output_dir_ = toml::find_or(main_table, "output", "./results");
            config.filename_mask_ = toml::find_or<std::vector<std::string>>(main_table, "filename_mask", std::vector<std::string>{"trade"});
            
        } catch (const std::exception& e) {
            spdlog::warn("TOML error: {}, using defaults", e.what());
            config.input_dir_ = "./data";
            config.output_dir_ = "./results";
            config.filename_mask_ = {"trade"};
        }
        
        spdlog::info("Config: input={} output={} mask=[{}]", 
                    config.input_dir_.string(), 
                    config.output_dir_.string(),
                    config.filename_mask_.size());
        return config;
    }
}
