#include "median_calculator.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>

namespace csv_median {
    void calculator::add_price(double price_) {
        prices_.push_back(price_);
    }

    std::optional<double> calculator::median() {
        if (prices_.empty()) return std::nullopt;
        
        auto sorted_ = prices_;
        std::nth_element(sorted_.begin(), sorted_.begin() + sorted_.size() / 2, sorted_.end());
        return sorted_[sorted_.size() / 2];
    }
}
