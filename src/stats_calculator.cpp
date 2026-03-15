/**
 * \file stats_calculator.cpp
 * \author Anastasiya Dorohina
 * \brief Boost.Accumulators: mean, std, median, p50/p90/p95/p99
 * \date 2026-03-15
 */

#include "stats_calculator.hpp"
#include <cmath>

namespace csv_median {

// Вероятности для extended_p_square: p50=0, p90=1, p95=2, p99=3
const std::vector<double> StatsCalculator::QUANTILE_PROBS = {
    0.50, 0.90, 0.95, 0.99
};

/**
 * \brief Конструктор: регистрирует запрошенные метрики и заголовки
 *
 * Инициализирует аккумулятор с фиксированным набором квантильных вероятностей.
 * Это позволяет обслуживать любой подмножество {p50, p90, p95, p99}
 * без перестройки типа аккумулятора в зависимости от конфига.
 *
 * \param metrics_ Список метрик из конфигурации
 */
StatsCalculator::StatsCalculator(const std::vector<std::string>& metrics_)
    : _acc(ba::extended_p_square_probabilities = QUANTILE_PROBS),
      _metrics(metrics_)
{
    _headers.reserve(metrics_.size() + 1);
    _headers.push_back("receive_ts");

    for (const auto& m : metrics_) {
        if (m == "mean")   { _headers.push_back("mean");    }
        else if (m == "std")    { _headers.push_back("std_dev"); }
        else if (m == "median") { _headers.push_back("median");  }
        else if (m == "p50")   { _headers.push_back("p50");     }
        else if (m == "p90")   { _headers.push_back("p90");     }
        else if (m == "p95")   { _headers.push_back("p95");     }
        else if (m == "p99")   { _headers.push_back("p99");     }
    }
}

/**
 * \brief Добавляет цену во все аккумуляторы
 * \param price_ Цена рыночного события
 */
void StatsCalculator::add_price(double price_) noexcept {
    _acc(price_);
}

/**
 * \brief Возвращает текущие значения всех запрошенных метрик
 *
 * Квантили извлекаются из extended_p_square по индексу:
 *  QUANTILE_PROBS = {0.50, 0.90, 0.95, 0.99}
 *  p50 → index 0, p90 → index 1, p95 → index 2, p99 → index 3
 *
 * \return Вектор optional<double> по числу метрик
 */
std::vector<std::optional<double>> StatsCalculator::get_stats() const {
    std::vector<std::optional<double>> result;
    result.reserve(_metrics.size());

    // Квантильные значения вычисляются один раз (O(1) при наличии данных)
    const auto& quantile_vals = ba::extended_p_square(_acc);

    for (const auto& m : _metrics) {
        if (m == "mean") {
            result.push_back(ba::mean(_acc));
        } else if (m == "std") {
            // variance() возвращает дисперсию; std = sqrt(variance)
            result.push_back(std::sqrt(ba::variance(_acc)));
        } else if (m == "median") {
            result.push_back(ba::median(_acc));
        } else if (m == "p50") {
            result.push_back(quantile_vals[0]);  // QUANTILE_PROBS[0] = 0.50
        } else if (m == "p90") {
            result.push_back(quantile_vals[1]);  // QUANTILE_PROBS[1] = 0.90
        } else if (m == "p95") {
            result.push_back(quantile_vals[2]);  // QUANTILE_PROBS[2] = 0.95
        } else if (m == "p99") {
            result.push_back(quantile_vals[3]);  // QUANTILE_PROBS[3] = 0.99
        } else {
            result.push_back(std::nullopt);
        }
    }

    return result;
}

/**
 * \brief Возвращает заголовки CSV
 * \return Заголовки в порядке метрик
 */
const std::vector<std::string>& StatsCalculator::get_headers() const noexcept {
    return _headers;
}

} // namespace csv_median
