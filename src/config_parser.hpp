#pragma once
#include <toml++/toml.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace fs = std::filesystem;

struct MainConfig {
    fs::path input_dir;
    fs::path output_dir;
    std::vector<std::string> filename_masks;
};

inline MainConfig parse_config(const fs::path& config_path) {
    if (!fs::exists(config_path)) {
        throw std::runtime_error(std::format("Config file not found: {}", config_path.string()));
    }

    auto table = toml::parse_file(config_path);
    auto main_tbl = toml::find<table>(table, "main");

    MainConfig cfg;
    cfg.input_dir = toml::get_or(main_tbl, "input", fs::path{});
    if (cfg.input_dir.empty()) {
        throw std::runtime_error("Missing required 'main.input'");
    }
    if (!fs::exists(cfg.input_dir)) {
        throw std::runtime_error(std::format("Input dir not exists: {}", cfg.input_dir.string()));
    }

    cfg.output_dir = toml::get_or(main_tbl, "output", fs::path{"output"});
    fs::create_directories(cfg.output_dir);

    cfg.filename_masks = toml::get_or<std::vector<std::string>>(main_tbl, "filename_mask", {});

    spdlog::info("Config loaded: input={}, output={}, masks={}", 
                 cfg.input_dir.string(), cfg.output_dir.string(), 
                 cfg.filename_masks.size());

    return cfg;
}
