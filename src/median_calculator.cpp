/**
 * \file median_calculator.cpp
 * \author Anastasiya Dorohina
 * \brief Реализация O(log N) медианы **без mutable**
 * \date 2026-03-15
 * \version 2.1
 */

#include "median_calculator.hpp"

namespace csv_median {

/**
 * \brief Разделение цены по текущей медиане **O(log N)**
 * 
 * **Логика двух куч:**
 * - price ≤ max(lower) → lower_half_.insert()
 * - Иначе → upper_half_.insert()  
 * - rebalance() восстанавливает инвариант размеров
 * 
 * \param[in] price Добавляемая цена
 */
void MedianCalculator::add_price(double price) {
    if (lower_half_.empty() || price <= *lower_half_.rbegin()) {
        lower_half_.insert(price);  // max-heap: rbegin()
    } else {
        upper_half_.insert(price);  // min-heap: begin()
    }
    rebalance();
}

/**
 * \brief Балансировка инварианта размеров **O(log N)**
 * 
 * **Инвариант ТЗ:** |lower| == |upper| ИЛИ |lower| == |upper| + 1
 * 
 * **Случаи:**
 * - |lower| > |upper| + 1 → max(lower) → upper
 * - |upper| > |lower| → min(upper) → lower
 * 
 * **Сохранение порядка:** max(lower) ≤ min(upper) всегда
 */
void MedianCalculator::rebalance() {
    if (lower_half_.size() > upper_half_.size() + 1) {
        upper_half_.insert(*lower_half_.rbegin());
        lower_half_.erase(std::prev(lower_half_.end()));
    } else if (upper_half_.size() > lower_half_.size()) {
        lower_half_.insert(*upper_half_.begin());
        upper_half_.erase(upper_half_.begin());
    }
}

/**
 * \brief Вычисление медианы с фильтром изменений **O(1)**
 * 
 * **Формулы медианы:**
 * - Чётное N: (max(lower) + min(upper)) / 2.0
 * - Нечётное N: max(lower)
 * 
 * **Фильтр ТЗ:** |current - last_median_| ≥ 1e-8 → запись
 * 
 * \return median при изменении или nullopt
 */
std::optional<double> MedianCalculator::median() {
    if (lower_half_.empty()) {
        return std::nullopt;
    }

    double current_median = (lower_half_.size() == upper_half_.size())
        ? (*lower_half_.rbegin() + *upper_half_.begin()) / 2.0
        : *lower_half_.rbegin();

    if (std::fabs(current_median - last_median_) >= 1e-8) {
        last_median_ = current_median;
        return current_median;
    }
    return std::nullopt;
}

} // namespace csv_median
