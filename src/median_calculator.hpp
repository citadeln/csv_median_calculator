/**
 * \file median_calculator.hpp
 * \author Anastasiya Dorohina
 * \brief O(log N) инкрементальная медиана через две кучи (lower/upper)
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
 * Алгоритм двух куч:
 *  - lower_half_: max-heap (нижняя половина <= медианы)
 *  - upper_half_: min-heap (верхняя половина >= медианы)
 *  - rebalance(): балансирует размеры куч (|lower| == |upper| или |lower| == |upper| + 1)
 *
 * median() возвращает значение только при изменении >= 1e-8 (ТЗ)
 */
class MedianCalculator {
public:
    /**
     * \brief Добавляет цену в медианный поток
     * \param price Новое значение цены
     */
    void add_price(double price);

    /**
     * \brief Возвращает медиану только при изменении
     * \return Новое значение медианы или nullopt если медиана не изменилась
     */
    [[nodiscard]] std::optional<double> median() const;

private:
    /**
     * \brief Балансировка размеров двух куч
     */
    void rebalance();

    std::multiset<double> lower_half_;      ///< Нижняя половина (max через rbegin)
    std::multiset<double> upper_half_;      ///< Верхняя половина (min через begin)
    mutable double last_median_ = 0.0;      ///< Предыдущее значение для сравнения
};

} // namespace csv_median
