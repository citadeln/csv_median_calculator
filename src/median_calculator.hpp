/**
 * \file median_calculator.hpp
 * \author Anastasiya Dorohina
 * \brief Инкрементальная медиана O(log N) через 2 кучи
 * \date 2026-03-08
 * \version 2.0
 */

#pragma once
#include <set>
#include <optional>
#include <cmath>

namespace csv_median {

/**
 * \class MedianCalculator
 * \brief Вычисляет медиану в O(log N) на потоке цен
 * 
 * Алгоритм двух куч (ТЗ):
 *  - lower_half_: max-heap (нижняя половина ≤ медиана)
 *  - upper_half_: min-heap (верхняя половина ≥ медиана)  
 *  - rebalance(): |lower| = |upper| или |lower| = |upper| + 1
 *  - median(): среднее верхнего(lower) и нижнего(upper) при чётности
 */
class MedianCalculator {
private:
    std::multiset<double> lower_half_;
    std::multiset<double> upper_half_;
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
    /**
     * \brief Добавляет цену в поток
     * \param price Новая цена события
     */
    void add_price(double price);

    /**
     * \brief Возвращает медиану если изменилась (ТЗ: 8 decimals)
     * \return median или nullopt если без изменений
     */    
    std::optional<double> median();
};

} // namespace csv_median
