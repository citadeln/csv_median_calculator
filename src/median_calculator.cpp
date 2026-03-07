#include "median_calculator.hpp"

namespace csv_median {

void MedianCalculator::add_price(double price) {
    if (lower_half_.empty() || price <= *lower_half_.rbegin()) {
        lower_half_.insert(price);
    } else {
        upper_half_.insert(price);
    }
    rebalance();
}

std::optional<double> MedianCalculator::median() {
    if (lower_half_.empty()) return std::nullopt;

    double current_median = (lower_half_.size() == upper_half_.size())
        ? (*lower_half_.rbegin() + *upper_half_.begin()) / 2.0
        : *lower_half_.rbegin();

    // ТЗ: изменение с точностью 8 знаков
    if (std::fabs(current_median - last_median_) >= 1e-8) {
        last_median_ = current_median;
        return current_median;
    }
    return std::nullopt;
}

} // namespace csv_median
