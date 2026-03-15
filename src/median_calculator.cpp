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
 * \brief Добавляет цену в медианный поток **O(log N)**
 * 
 * **Алгоритм двух куч (ТЗ):**
 * 1. price <= max(lower_half_) → вставка в lower_half_ (max-heap через rbegin)
 * 2. Иначе → вставка в upper_half_ (min-heap через begin)  
 * 3. rebalance() для инварианта: |lower| == |upper| ИЛИ |lower| == |upper| + 1
 * 
 * **Инварианты после add_price():**
 * - Все элементы lower_half_ <= все элементы upper_half_
 * - Размеры куч сбалансированы
 * 
 * \param price Новое значение цены для добавления в поток
 * \Complexity O(log N) вставка + O(log N) rebalance
 */
void MedianCalculator::add_price(double price) {
    // Разделение по медиане: нижняя/верхняя половина
    if (lower_half_.empty() || price <= *lower_half_.rbegin()) {
        lower_half_.insert(price);  // max-heap: rbegin() = максимум
    } else {
        upper_half_.insert(price);  // min-heap: begin() = минимум
    }
    rebalance();  // Восстановление баланса размеров
}

/**
 * \brief Балансировка размеров двух куч **O(log N)**
 * 
 * **Логика балансировки (строго ТЗ инвариант):**
 * - **Случай 1:** |lower| > |upper| + 1 → max(lower) → upper
 * - **Случай 2:** |upper| > |lower| → min(upper) → lower  
 * - Иначе баланс соблюдён
 * 
 * **Почему это правильно:**
 * - max(lower) всегда <= min(upper) → инвариант сохраняется
 * - После баланса: |lower| == |upper| или |lower| == |upper| + 1
 */
void MedianCalculator::rebalance() {
    // lower переполнен: перемещаем максимум в upper
    if (lower_half_.size() > upper_half_.size() + 1) {
        upper_half_.insert(*lower_half_.rbegin());
        lower_half_.erase(std::prev(lower_half_.end()));
    }
    // upper больше lower: перемещаем минимум в lower  
    else if (upper_half_.size() > lower_half_.size()) {
        lower_half_.insert(*upper_half_.begin());
        upper_half_.erase(upper_half_.begin());
    }
    // Иначе: |lower| == |upper| или |lower| == |upper| + 1 ✓
}

/**
 * \brief Возвращает медиану **только при изменении** >= 1e-8
 * 
 * **Формула медианы (ТЗ):**
 * - |lower| == |upper|: (max(lower) + min(upper)) / 2.0
 * - |lower| == |upper| + 1: max(lower)
 * 
 * **Фильтр изменений (ТЗ):** 
 * - Сравнение с last_median_ по модулю с точностью 8 знаков (1e-8)
 * - Изменение → обновление last_median_ + возврат значения
 * - Без изменения → std::nullopt (пропуск записи)
 * 
 * \return Новое значение медианы или nullopt (нет данных/без изменений)
 * \Complexity O(1) — только чтение begin/rbegin
 */
std::optional<double> MedianCalculator::median() const {
    // Нет данных → ТЗ: nullopt
    if (lower_half_.empty()) {
        return std::nullopt;
    }

    // Вычисление текущей медианы по инварианту куч
    double current_median = (lower_half_.size() == upper_half_.size())
        ? (*lower_half_.rbegin() + *upper_half_.begin()) / 2.0  // Чётное: среднее двух средних
        : *lower_half_.rbegin();                                  // Нечётное: среднее lower

    // ТЗ: запись только при изменении >= 8 знаков точности
    if (std::fabs(current_median - last_median_) >= 1e-8) {
        last_median_ = current_median;  // Обновление для следующего сравнения
        return current_median;
    }
    return std::nullopt;  // Медиана не изменилась → пропуск записи
}

} // namespace csv_median
