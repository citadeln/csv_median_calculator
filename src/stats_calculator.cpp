#include "stats_calculator.hpp"
#include <spdlog/spdlog.h>
#include <boost/accumulators/statistics/median.hpp>

namespace csv_median {

StatsCalculator::StatsCalculator(const std::vector<std::string>& metrics) 
    : metrics_(metrics) {
    
    // Генерация заголовков колонок
    headers_ = {"receive_ts"};
    for (const auto& m : metrics) {
        if (m == "mean") headers_.push_back("mean");
        else if (m == "std") headers_.push_back("std_dev");
        else if (m == "p50") headers_.push_back("p50");
        else if (m == "p90") headers_.push_back("p90");
        else if (m == "p95") headers_.push_back("p95");
        else if (m == "p99") headers_.push_back("p99");
        else if (m == "median") headers_.push_back("median");  // legacy
    }
}

void StatsCalculator::add_price(double price) {
    acc_(price);
}

std::vector<std::optional<double>> StatsCalculator::get_stats() const {
    std::vector<std::optional<double>> stats;
    
    for (const auto& m : metrics_) {
        if (m == "mean") {
            stats.push_back(mean(acc_));
        } else if (m == "std") {
            stats.push_back(sqrt(variance(acc_)));
        } else if (m == "p50") {
            stats.push_back(median(acc_));
        } else if (m == "p90") {
            stats.push_back(boost::accumulators::quantile(acc_, quantile_probability<0.9>()));
        } else if (m == "p95") {
            stats.push_back(boost::accumulators::quantile(acc_, quantile_probability<0.95>()));
        } else if (m == "p99") {
            stats.push_back(boost::accumulators::quantile(acc_, quantile_probability<0.99>()));
        } else if (m == "median") {
            stats.push_back(median(acc_));
        } else {
            stats.push_back(std::nullopt);
        }
    }
    return stats;
}

std::vector<std::string> StatsCalculator::get_headers() const {
    return headers_;
}

}
