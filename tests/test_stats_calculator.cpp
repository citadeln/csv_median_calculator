/**
 * \file test_stats_calculator.cpp
 * \author Anastasiya Dorohina
 * \brief Unit-тесты Boost.Accumulators ТЗ 7.2
 * \date 2026-03-15
 */

#define BOOST_TEST_MODULE StatsCalculatorTests
#include "../src/stats_calculator.hpp"
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <vector>

// Тестовые данные: {68480.0, 68479.9, 68480.1, 68480.05, 68481.0}
// Точная медиана = 68480.05, mean = 68480.21, std ≈ 0.4005
static const std::vector<double> TEST_PRICES = {
    68480.00000000,
    68479.90000000,
    68480.10000000,
    68480.05000000,
    68481.00000000
};

// Расширенный набор для квантилей — P-squared требует ≥50 точек для сходимости
static std::vector<double> make_price_series(int n) {
    std::vector<double> prices;
    prices.reserve(n);
    for (int i = 0; i < n; ++i) {
        prices.push_back(68480.0 + (i % 10) * 0.01);
    }
    return prices;
}

BOOST_AUTO_TEST_SUITE(StatsCalculatorTests)

// Проверяем mean и std_dev на 5 точках
// mean = (68480.00 + 68479.90 + 68480.10 + 68480.05 + 68481.00) / 5 = 68480.21
// variance (population) = sum((x - mean)^2) / n = 0.8020 / 5 = 0.16040
// std_dev = sqrt(0.16040) ≈ 0.40050
BOOST_AUTO_TEST_CASE(Test_Mean_And_Std) {
    csv_median::StatsCalculator calc({"mean", "std"});

    for (double price : TEST_PRICES) {
        calc.add_price(price);
    }

    auto stats = calc.get_stats();
    BOOST_REQUIRE_EQUAL(stats.size(), 2u);

    BOOST_REQUIRE(stats[0].has_value());
    BOOST_REQUIRE(stats[1].has_value());

    BOOST_CHECK_CLOSE(*stats[0], 68480.21, 1e-4);   // mean
    BOOST_CHECK_CLOSE(*stats[1], 0.4005,   1e-2);   // std_dev ≈ 0.4005
}

// Boost P-squared медиана: приближённая, на 5 точках допуск 1%
BOOST_AUTO_TEST_CASE(Test_Boost_Median_Approx) {
    csv_median::StatsCalculator calc({"median"});

    for (double price : TEST_PRICES) {
        calc.add_price(price);
    }

    auto stats = calc.get_stats();
    BOOST_REQUIRE_EQUAL(stats.size(), 1u);
    BOOST_REQUIRE(stats[0].has_value());

    // Точная медиана = 68480.05; P-squared на малых выборках — допуск 1%
    BOOST_CHECK_CLOSE(*stats[0], 68480.05, 1.0);
}

// Квантили P-squared сходятся на достаточном объёме данных (≥50 точек)
BOOST_AUTO_TEST_CASE(Test_Percentiles_Convergence) {
    csv_median::StatsCalculator calc({"p50", "p90", "p95", "p99"});

    const auto prices = make_price_series(100);
    for (double price : prices) {
        calc.add_price(price);
    }

    auto stats = calc.get_stats();
    BOOST_REQUIRE_EQUAL(stats.size(), 4u);

    for (const auto& s : stats) {
        BOOST_REQUIRE(s.has_value());
        // Все перцентили должны быть в разумном диапазоне цен
        BOOST_CHECK_GE(*s, 68479.0);
        BOOST_CHECK_LE(*s, 68482.0);
    }

    // P50 ≤ P90 ≤ P95 ≤ P99 (монотонность квантилей)
    BOOST_CHECK_LE(*stats[0], *stats[1]);
    BOOST_CHECK_LE(*stats[1], *stats[2]);
    BOOST_CHECK_LE(*stats[2], *stats[3]);
}

// Граничный случай: одна цена
BOOST_AUTO_TEST_CASE(Test_Single_Price) {
    csv_median::StatsCalculator calc({"mean", "std"});

    calc.add_price(68480.10);

    auto stats = calc.get_stats();
    BOOST_REQUIRE_EQUAL(stats.size(), 2u);

    BOOST_REQUIRE(stats[0].has_value());
    BOOST_CHECK_CLOSE(*stats[0], 68480.10, 1e-6);   // mean == price

    BOOST_REQUIRE(stats[1].has_value());
    BOOST_CHECK_SMALL(*stats[1], 1e-8);              // std_dev == 0
}

// Пустой список метрик — get_stats() возвращает пустой вектор
BOOST_AUTO_TEST_CASE(Test_Empty_Metrics) {
    csv_median::StatsCalculator calc({});

    for (double price : TEST_PRICES) {
        calc.add_price(price);
    }

    BOOST_CHECK(calc.get_stats().empty());
}

// Заголовки: "receive_ts" + имена метрик в порядке конфига
BOOST_AUTO_TEST_CASE(Test_Headers_Order) {
    csv_median::StatsCalculator calc({"mean", "p99", "median"});

    const auto& headers = calc.get_headers();
    BOOST_REQUIRE_EQUAL(headers.size(), 4u);
    BOOST_CHECK_EQUAL(headers[0], "receive_ts");
    BOOST_CHECK_EQUAL(headers[1], "mean");
    BOOST_CHECK_EQUAL(headers[2], "p99");
    BOOST_CHECK_EQUAL(headers[3], "median");
}

BOOST_AUTO_TEST_SUITE_END()
