/**
 * \file stats_calculator.hpp
 * \author Anastasiya Dorohina
 * \brief Boost.Accumulators: mean, std, median, p50/p90/p95/p99
 * \date 2026-03-15
 */

#pragma once

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/extended_p_square.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <optional>
#include <string>
#include <vector>

namespace csv_median {

namespace ba = boost::accumulators;

/**
 * \class StatsCalculator
 * \brief Инкрементальный расчёт статистических метрик через Boost.Accumulators
 *
 * Поддерживаемые метрики (задаются через config.metrics):
 *  - "median" -- медиана (P-squared алгоритм)
 *  - "mean"   -- среднее значение
 *  - "std"    -- стандартное отклонение
 *  - "p50"    -- 50-й перцентиль (extended P-squared)
 *  - "p90"    -- 90-й перцентиль
 *  - "p95"    -- 95-й перцентиль
 *  - "p99"    -- 99-й перцентиль
 *
 * Квантили рассчитываются через extended_p_square с фиксированным набором
 * вероятностей {0.5, 0.9, 0.95, 0.99}, инициализируемых в конструкторе.
 */
class StatsCalculator {
public:
    /**
     * \brief Конструктор: регистрирует запрошенные метрики и заголовки
     * \param metrics_ Список метрик из конфигурации
     */
    explicit StatsCalculator(const std::vector<std::string>& metrics_);

    /**
     * \brief Добавляет цену во все аккумуляторы
     * \param price_ Цена рыночного события
     */
    void add_price(double price_) noexcept;

    /**
     * \brief Возвращает текущие значения всех запрошенных метрик
     *
     * Порядок значений соответствует порядку метрик, переданных в конструктор.
     * Возвращает nullopt для неизвестных метрик.
     *
     * \return Вектор optional<double> по числу метрик
     */
    [[nodiscard]] std::vector<std::optional<double>> get_stats() const;

    /**
     * \brief Возвращает заголовки CSV: "receive_ts" + имена метрик
     * \return Заголовки в порядке метрик
     */
    [[nodiscard]] const std::vector<std::string>& get_headers() const noexcept;

private:
    /**
     * \brief Фиксированный набор вероятностей для extended_p_square
     *
     * Все квантильные запросы (p50/p90/p95/p99) обслуживаются одним
     * набором аккумуляторов.
     *
     * Индексы: p50=0, p90=1, p95=2, p99=3
     */
    static const std::vector<double> QUANTILE_PROBS;

    using acc_set_t = ba::accumulator_set<double, ba::stats<
        ba::tag::mean,
        ba::tag::variance,
        ba::tag::median,
        ba::tag::extended_p_square
    >>;

    acc_set_t _acc;                      ///< Boost.Accumulators набор
    std::vector<std::string> _metrics;   ///< Запрошенные метрики
    std::vector<std::string> _headers;   ///< Заголовки CSV
};

} // namespace csv_median
