/**
 * \file median_calculator.cpp
 * \author Anastasiya Dorohina
 * \brief O(log N) медиана через две кучи (lower/upper)
 * \date 2026-03-08
 * \version 2.0
 */

#include "median_calculator.hpp"

namespace csv_median {

/**
 * \brief Добавляет цену в поток событий
 * 
 * Логика:
 *  - price ≤ верхняя граница lower_half_ → lower_half_
 *  - Иначе → upper_half_
 *  - rebalance() для инварианта куч
 * 
 * \param price Цена рыночного события
 */
void MedianCalculator::add_price(double price) {
    if (lower_half_.empty() || price <= *lower_half_.rbegin()) {
        lower_half_.insert(price);
    } else {
        upper_half_.insert(price);
    }
    rebalance();
}

/**
 * \brief Вычисляет медиану с дедупликацией изменений (ТЗ 8 decimals)
 * 
 * Медиана:
 *  - |lower| == |upper|: (max(lower) + min(upper)) / 2
 *  - |lower| = |upper| + 1: max(lower)
 * 
 * Возвращает только при изменении ≥ 1e-8
 * 
 * \return median или nullopt (без изменений)
 */
std::optional<double> MedianCalculator::median() {
    if (lower_half_.empty()) return std::nullopt;

    double current_median = (lower_half_.size() == upper_half_.size())
        ? (*lower_half_.rbegin() + *upper_half_.begin()) / 2.0
        : *lower_half_.rbegin();

    // ТЗ: изменения с точностью 8 знаков
    if (std::fabs(current_median - last_median_) >= 1e-8) {
        last_median_ = current_median;
        return current_median;
    }
    return std::nullopt;
}

} // namespace csv_median
