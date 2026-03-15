#pragma once
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <vector>
#include <string>
#include <optional>

namespace csv_median {
using namespace boost::accumulators;

class StatsCalculator {
public:
    StatsCalculator(const std::vector<std::string>& metrics);
    void add_price(double price);
    std::vector<std::optional<double>> get_stats() const;
    std::vector<std::string> get_headers() const;

private:
    // Упрощенный accumulator_set только с базовыми stats
    accumulator_set<double, stats<
    tag::mean, 
    tag::variance, 
    tag::median,
    tag::quantile<tag::quantile_probability<0.9>>,
    tag::quantile<tag::quantile_probability<0.95>>, 
    tag::quantile<tag::quantile_probability<0.99>>
> > acc_;
    
    std::vector<std::string> metrics_;
    std::vector<std::string> headers_;
};
}
