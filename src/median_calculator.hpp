#pragma once
#include <set>
#include <optional>
#include <cmath>

namespace csv_median {

class MedianCalculator {
private:
    std::multiset<double> lower_half_;  // max-heap (нижняя половина)
    std::multiset<double> upper_half_;  // min-heap (верхняя половина)
    double last_median_ = 0.0;

    void rebalance() {
        if (lower_half_.size() > upper_half_.size() + 1) {
            upper_half_.insert(*lower_half_.rbegin());
            lower_half_.erase(std::prev(lower_half_.end()));
        } else if (upper_half_.size() > lower_half_.size()) {
            lower_half_.insert(*upper_half_.begin());
            upper_half_.erase(upper_half_.begin());
        }
    }

public:
    void add_price(double price);
    std::optional<double> median();
};

} // namespace csv_median
