/**
 * \file main.cpp
 * \author Anastasiya Dorohina
 * \brief Запись при изменении медианы (точный расчёт), вывод всех метрик
 * \date 2026-03-15
 * \version 2.3
 */

#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "median_calculator.hpp"
#include "stats_calculator.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ranges>
#include <boost/program_options.hpp>

namespace fs = std::filesystem;
namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::info("csv_median_calculator v2.3 C++23 multi-metrics");

    try {
        po::options_description desc("Options");
        desc.add_options()
            ("config", po::value<std::string>(), "Path to config.toml")
            ("cfg",    po::value<std::string>(), "Path to config.toml (alias)");

        po::variables_map vm;
        po::store(
            po::command_line_parser(argc, argv)
                .options(desc)
                .style(po::command_line_style::allow_long
                     | po::command_line_style::long_allow_adjacent
                     | po::command_line_style::allow_dash_for_short)
                .run(),
            vm);
        po::notify(vm);

        fs::path config_path;
        if (vm.count("config")) {
            config_path = vm["config"].as<std::string>();
        } else if (vm.count("cfg")) {
            config_path = vm["cfg"].as<std::string>();
        } else {
            config_path = fs::path(argv[0]).parent_path() / "config.toml";
        }

        spdlog::info("Using config: {}", config_path.string());
        auto config = csv_median::parse_config(config_path);

        auto events = csv_median::read_csv_files(config.input_dir, config.filename_mask);
        spdlog::info("Total events read: {}", events.size());

        if (events.empty()) {
            spdlog::warn("No events found, output file will contain only header");
        }

        // ТЗ: stable_sort по receive_ts сохраняет относительный порядок при равных ts
        std::ranges::stable_sort(events, {}, &csv_median::MarketEvent::receive_ts);
        spdlog::info("Sorted {} events by receive_ts", events.size());

        auto output_path = config.output_dir / "median_result.csv";
        std::ofstream out(output_path);
        if (!out.is_open()) {
            spdlog::error("Cannot create output file: {}", output_path.string());
            return 1;
        }

        // MedianCalculator: точный O(log N) двухкучевой алгоритм.
        // Используется для: (1) детектирования изменения медианы, (2) точного значения медианы в строке.
        csv_median::MedianCalculator median_calc;

        // StatsCalculator: Boost.Accumulators для mean, std, p50/p90/p95/p99.
        // НЕ используется для медианы — только для дополнительных метрик.
        csv_median::StatsCalculator stats_calc(config.metrics);

        // Заголовки из StatsCalculator: "receive_ts;median;mean;std_dev;..."
        // (порядок определяется config.metrics)
        const auto& header_row = stats_calc.get_headers();
        for (size_t i = 0; i < header_row.size(); ++i) {
            if (i > 0) { out << ";"; }
            out << header_row[i];
        }
        out << "\n";
        out << std::fixed << std::setprecision(8);

        int change_count = 0;

        for (const auto& event : events) {
            // Оба калькулятора получают одну и ту же цену синхронно
            median_calc.add_price(event.price);
            stats_calc.add_price(event.price);

            // ТЗ: строка записывается только при изменении ТОЧНОЙ медианы.
            // median_calc.median() возвращает nullopt если медиана не изменилась.
            if (const auto median_opt = median_calc.median()) {
                out << event.receive_ts;

                // Итерируем метрики в порядке config.metrics:
                // - "median" -> точное значение из MedianCalculator (*median_opt)
                // - остальные -> значения из StatsCalculator
                const auto stats = stats_calc.get_stats();
                for (size_t i = 0; i < config.metrics.size(); ++i) {
                    if (config.metrics[i] == "median") {
                        // Точная медиана — двухкучевой алгоритм
                        out << ";" << *median_opt;
                    } else {
                        // Boost.Accumulators: mean, std, p50/p90/p95/p99
                        out << ";" << (stats[i].has_value() ? *stats[i] : 0.0);
                    }
                }
                out << "\n";
                ++change_count;
            }
        }

        out.close();
        spdlog::info("Saved {} rows (median changes) to {}",
                     change_count, output_path.string());
        spdlog::info("Done!");

    } catch (const po::error& ex) {
        spdlog::error("CLI error: {}", ex.what());
        return 1;
    } catch (const std::exception& ex) {
        spdlog::error("Exception: {}", ex.what());
        return 1;
    } catch (...) {
        spdlog::critical("Unknown error");
        return 255;
    }

    return 0;
}
