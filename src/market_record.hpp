#pragma once
#include <string>
#include <chrono>

namespace csv_median {

struct MarketEvent {
    uint64_t receive_ts;  // микросекунды (ТЗ)
    double price;
    
    auto operator<=>(const MarketEvent&) const = default;
};

} // namespace csv_median
