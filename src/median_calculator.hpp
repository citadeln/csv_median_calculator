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
