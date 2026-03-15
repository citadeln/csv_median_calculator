/**
 * \file median_calculator.hpp
 * \author Anastasiya Dorohina
 * \brief O(log N) инкрементальная медиана через две кучи **C++23**
 * \date 2026-03-15
 * \version 2.1
 */

#pragma once

#include <set>
#include <optional>
#include <cmath>

namespace csv_median {

/**
 * \class MedianCalculator
 * \brief **O(log N)** инкрементальная медиана двух кучами
 * 
 * **Алгоритм (ТЗ):**
 * - lower_half_ (multiset): max-heap, нижняя половина ≤ медианы
 * - upper_half_ (multiset): min-heap, верхняя половина ≥ медианы  
 * - Инвариант: |lower| == |upper| ИЛИ |lower| == |upper| + 1
 * - median(): возврат **только при изменении** ≥ 1e-8 (8 знаков ТЗ)
 * 
 * **Сложность:**
 * - add_price(): O(log N)
 * - median(): O(1)
 * - Память: O(N)
 */
class MedianCalculator {
public:
    /**
     * \brief Добавляет цену в поток **O(log N)**
     * \param[in] price Цена для добавления
     */
    void add_price(double price);

    /**
     * \brief Медиана при изменении ≥ 1e-8 **O(1)**
     * 
     * **Логика (ТЗ):**
     * - |lower| == |upper|: (max(lower) + min(upper)) / 2
     * - |lower| == |upper| + 1: max(lower)
     * - Изменение ≥ 1e-8 → обновление last_median_ + возврат
     * 
     * \return median или nullopt (нет изменений/данных)
     */
    [[nodiscard]] std::optional<double> median();

private:
    /**
     * \brief Балансировка куч **O(log N)**
     * Восстанавливает инвариант размеров куч
     */
    void rebalance();
    
    std::multiset<double> lower_half_;  ///< max-heap (rbegin)
    std::multiset<double> upper_half_;  ///< min-heap (begin)
    double last_median_ = 0.0;          ///< Для фильтрации изменений 1e-8
};

} // namespace csv_median
