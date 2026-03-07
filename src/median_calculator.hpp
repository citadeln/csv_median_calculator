/// \file median_calculator.hpp
/// \brief Hybrid median: nth_element (exact) + P² (streaming O(1))
#pragma once
#include <optional>
#include <vector>

namespace csv_median {
    class calculator {
        std::vector<double> prices_;
    public:
        void add_price(double price_);
        std::optional<double> median();
    };
}
