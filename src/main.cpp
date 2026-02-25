#include <iostream>
#include <boost/program_options.hpp>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <algorithm>
#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "median_calculator.hpp"

namespace po = boost::program_options;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    try {
        po::options_description desc("Options");
        po::positional_options_description p;
        fs::path config_path;

        desc.add_options()
            ("config,c", po::value<fs::path>(&config_path)->value_name("PATH"), "Path to config.toml")
            ("cfg", po::value<fs::path>(&config_path)->value_name("PATH"), "Path to config.toml (alias)");

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (!vm.count("config") && !vm.count("cfg")) {
            config_path = fs::path(argv[0]).parent_path() / "config.toml";
        }

        spdlog::info("Запуск csv_median_calculator v1.0.0");

        auto cfg = parse_config(config_path);

        // Scan files
        std::vector<MarketRecord> all_records;
        std::vector<fs::path> processed_files;
        for (const auto& entry : fs::directory_iterator(cfg.input_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                const auto& fname = entry.path().filename().string();
                bool matches = cfg.filename_masks.empty();
                for (const auto& mask : cfg.filename_masks) {
                    if (fname.find(mask) != std::string::npos) {
                        matches = true;
                        break;
                    }
                }
                if (matches) {
                    auto records = read_csv_file(entry.path());
                    if (!records.empty()) {
                        all_records.insert(all_records.end(), std::make_move_iterator(records.begin()), 
                                         std::make_move_iterator(records.end()));
                        processed_files.push_back(entry.path());
                    }
                }
            }
        }

        spdlog::info("Найдено {} файлов", processed_files.size());
        for (const auto& f : processed_files) {
            spdlog::info("  - {}", f.filename().string());
        }

        if (all_records.empty()) {
            spdlog::warn("No valid data found");
            return 0;
        }

        std::stable_sort(all_records.begin(), all_records.end());

        spdlog::info("Всего записей: {}", all_records.size());

        // Calculate median incrementally
        RunningMedian median;
        fs::path out_path = cfg.output_dir / "median_result.csv";
        std::ofstream out(out_path);
        out << "receive_ts;price_median\n";

        size_t changes = 0;
        for (const auto& rec : all_records) {
            median.add(rec.price);
            if (median.median_changed(rec.receive_ts, out)) {
                ++changes;
            }
        }

        spdlog::info("Изменений медианы: {}", changes);
        spdlog::info("Результат: {}", out_path.string());
        spdlog::info("Завершение работы");

    } catch (const std::exception& e) {
        spdlog::error("Ошибка: {}", e.what());
        return 1;
    }

    return 0;
}
