/**
 * \file market_record.hpp
 * \author Anastasiya Dorohina
 * \brief Структуры рыночных событий CSV
 * \date 2026-03-08
 * \version 2.0
 */

#pragma once
#include <string>
#include <chrono>

namespace csv_median {

/**
 * \struct MarketEvent
 * \brief Рыночное событие из CSV файла
 */
struct MarketEvent {
    uint64_t receive_ts;  ///< Timestamp получения (микросекунды)
    double   price;       ///< Цена инструмента
    
    auto operator<=>(const MarketEvent&) const = default;
};

} // namespace csv_median
