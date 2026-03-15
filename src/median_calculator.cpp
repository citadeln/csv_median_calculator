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
 * \brief Добавляет цену в медианный поток
 *
 * \param price Новое значение цены
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
 * \brief Балансировка размеров двух куч
 *
 * Восстанавливает инвариант: |lower| == |upper| или |lower| == |upper| + 1
 */
void MedianCalculator::rebalance() {
    // lower переполнен — перемещаем максимум в upper
    if (lower_half_.size() > upper_half_.size() + 1) {
        upper_half_.insert(*lower_half_.rbegin());
        lower_half_.erase(std::prev(lower_half_.end()));
    }
    // upper больше lower — перемещаем минимум в lower
    else if (upper_half_.size() > lower_half_.size()) {
        lower_half_.insert(*upper_half_.begin());
        upper_half_.erase(upper_half_.begin());
    }
}

/**
 * \brief Возвращает медиану только при изменении
 *
 * \return Новое значение медианы или nullopt если данных нет или медиана не изменилась
 */
std::optional<double> MedianCalculator::median() const noexcept {
    if (lower_half_.empty()) {
        return std::nullopt;
    }

    double current_median = (lower_half_.size() == upper_half_.size())
        ? (*lower_half_.rbegin() + *upper_half_.begin()) / 2.0
        : *lower_half_.rbegin();

    // ТЗ: изменение определяется с точностью до 8 знаков после запятой
    if (std::fabs(current_median - last_median_) >= 1e-8) {
        last_median_ = current_median;
        return current_median;
    }
    return std::nullopt;
}

} // namespace csv_median
