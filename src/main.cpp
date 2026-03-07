#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "median_calculator.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <boost/program_options.hpp>

namespace fs = std::filesystem;
namespace po = boost::program_options;

int main(int argc_, char* argv_[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::info("🚀 csv_median_calculator v2.0 C++23");

    try {
        // ТЗ: ./app --config config.toml
        po::options_description desc("Options");
        desc.add_options()
            ("config,c", po::value<std::string>(), "config.toml path")
            ("help,h", "show help");

        po::variables_map vm;
        po::store(po::parse_command_line(argc_, argv_, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }

        fs::path config_path_ = vm["config"].as<std::string>();
        auto config_ = csv_median::parse_config(config_path_);
        
        // ТЗ: output_dir или ./output
        if (config_.output_dir_.empty()) {
            config_.output_dir_ = "./output";
        }
        fs::create_directories(config_.output_dir_);

        auto events_ = csv_median::read_csv_files(config_.input_dir_, config_.filename_mask_);
        
        csv_median::calculator calc_;
        for (const auto& event_ : events_) {
            calc_.add_price(event_.price);
        }

        // ТЗ: median_result.csv + 8 decimals + semicolon
        auto output_path_ = config_.output_dir_ / "median_result.csv";
        std::ofstream out_(output_path_);
        out_ << "receive_ts;price_median\n";
        out_.precision(8);
        out_.setf(std::ios::fixed);
        
        if (auto median_ = calc_.median()) {
            out_ << events_.back().receive_ts << ";" << *median_ << "\n";
            spdlog::info("💾 {}: {:.8f}", output_path_.string(), *median_);
        }

    } catch (const std::exception& ex_) {
        spdlog::error("💥 {}", ex_.what());
        return 1;
    }
    return 0;
}
