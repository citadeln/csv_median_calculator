#include "config_parser.hpp"
#include <toml++/toml.hpp>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace csv_median {
namespace fs = std::filesystem;

Config parse_config(const fs::path& path) {
    Config config;

    // Дефолтные значения ТЗ
    config.input_dir = "./data";
    config.filename_mask = {"level", "trade"};
    config.output_dir = "./output";

    if (!fs::exists(path)) {
        spdlog::warn("⚠️ Config {} not found, using defaults", path.string());
        fs::create_directories(config.output_dir);
        return config;
    }

    try {
        auto data = toml::parse_file(path.string());

        if (auto main_table = data["main"]; main_table && main_table.is_table()) {
            // input_dir: string → path
            if (auto input_val = main_table["input"]; input_val && input_val.is_string()) {
                config.input_dir = fs::path(input_val.as_string()->get());
            }

            // output_dir: string → path
            if (auto output_val = main_table["output"]; output_val && output_val.is_string()) {
                config.output_dir = fs::path(output_val.as_string()->get());
            }

            // filename_mask: array → vector<string>
            if (auto mask_array = main_table["filename_mask"]; mask_array && mask_array.is_array()) {
                config.filename_mask.clear();
                for (const auto& item : *mask_array.as_array()) {
                    if (item.is_string()) {
                        config.filename_mask.push_back(item.as_string()->get());
                    }
                }
            }

            // Логи без fmt::join
            spdlog::info("✅ TOML parsed: input='{}' mask=[{}]",
                         config.input_dir.string(),
                         config.filename_mask.empty() ? "none" :
                         (config.filename_mask.size() > 1 ?
                          config.filename_mask[0] + ", " + config.filename_mask[1] :
                          config.filename_mask[0]));
        }
    } catch (const std::exception& e) {
        spdlog::warn("❌ TOML error '{}', using defaults", e.what());
    }

    fs::create_directories(config.output_dir);
    return config;
}

} // namespace csv_median
