#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "median_calculator.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <boost/program_options.hpp>
#include <algorithm>  // std::stable_sort
#include <ranges>

namespace fs = std::filesystem;
namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::info("🚀 csv_median_calculator v2.0 C++23");

    try {
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

        // ТЗ: --config config.toml или config.toml в текущей папке
        fs::path config_path = vm.count("config") ? 
            fs::path(vm["config"].as<std::string>()) : "config.toml";
            
        auto config = csv_median::parse_config(config_path);
        fs::create_directories(config.output_dir);

        // Читаем CSV файлы
        auto events = csv_median::read_csv_files(config.input_dir, config.filename_mask);
        
        // ТЗ: std::stable_sort по receive_ts
        std::stable_sort(events.begin(), events.end(), 
            [](const auto& a, const auto& b) { return a.receive_ts < b.receive_ts; });
        
        spdlog::info("📊 Total events: {}", events.size());

        // Инкрементальная медиана (ТЗ)
        csv_median::MedianCalculator calc;
        auto output_path = config.output_dir / "median_result.csv";
        std::ofstream out(output_path);
        out << "receive_ts;price_median\n";
        out.precision(8);
        out << std::fixed;

        double prev_median = 0.0;
        for (const auto& event : events) {
            calc.add_price(event.price);
            if (auto median = calc.median()) {
                out << event.receive_ts << ";" << *median << "\n";
                prev_median = *median;
                spdlog::debug("📈 {}: {:.8f}", event.receive_ts, *median);
            }
        }

        spdlog::info("💾 Saved: {} ({} changes)", output_path.string(), /* count changes */);

    } catch (const std::exception& ex) {
        spdlog::error("💥 {}", ex.what());
        return 1;
    }
    return 0;
}
