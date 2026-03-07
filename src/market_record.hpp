#pragma once
#include <cstdint>

namespace csv_median {
    struct market_record_t {
        uint64_t receive_ts;
        double price;
    };
}
