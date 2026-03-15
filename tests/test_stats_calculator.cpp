/**
 * \file test_stats_calculator.cpp
 * \author Anastasiya Dorohina  
 * \brief Unit-тесты Boost.Accumulators ТЗ 7.2
 * \date 2026-03-15
 */

#include "stats_calculator.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <cmath>

// Тестовые данные: {68480.0, 68479.9, 68480.1, 68480.05, 68481.0}
static const std::vector<double> TEST_PRICES = {
    68480.00000000,
    68479.90000000, 
    68480.10000000,
    68480.05000000,
    68481.00000000
};

BOOST_AUTO_TEST_SUITE(StatsCalculatorTests)

BOOST_AUTO_TEST_CASE(Test_Metrics_Median_Mean_Std) {
    // config.metrics = ["median", "mean", "std"]
    std::vector<std::string> metrics = {"median", "mean", "std"};
    csv_median::StatsCalculator calc(metrics);
    
    // Добавляем тестовые цены
    for (double price : TEST_PRICES) {
        calc.add_price(price);
    }
    
    auto stats = calc.get_stats();
    BOOST_REQUIRE_EQUAL(stats.size(), 3);
    
    // Проверяем значения (с точностью 1e-8)
    BOOST_CHECK_CLOSE(*stats[0], 68480.05, 1e-6);  // median
    BOOST_CHECK_CLOSE(*stats[1], 68480.21, 1e-6);  // mean
    BOOST_CHECK_CLOSE(*stats[2], 0.0433, 1e-4);    // std_dev
}

BOOST_AUTO_TEST_CASE(Test_Percentiles_P50_P90_P95_P99) {
    std::vector<std::string> metrics = {"p50", "p90", "p95", "p99"};
    csv_median::StatsCalculator calc(metrics);
    
    for (double price : TEST_PRICES) {
        calc.add_price(price);
    }
    
    auto stats = calc.get_stats();
    BOOST_REQUIRE_EQUAL(stats.size(), 4);
    
    // P50 ≈ median = 68480.05
    BOOST_CHECK_CLOSE(*stats[0], 68480.05, 1e-6);
    // P90 ≈ 68480.95 (90% квантиль)
    BOOST_CHECK_CLOSE(*stats[1], 68480.95, 1e-4);
}

BOOST_AUTO_TEST_CASE(Test_Empty_Metrics) {
    // Пустой список метрик
    std::vector<std::string> metrics;
    csv_median::StatsCalculator calc(metrics);
    
    for (double price : TEST_PRICES) {
        calc.add_price(price);
    }
    
    auto stats = calc.get_stats();
    BOOST_CHECK(stats.empty());
}

BOOST_AUTO_TEST_CASE(Test_Headers_Generation) {
    std::vector<std::string> metrics = {"mean", "p99", "median"};
    csv_median::StatsCalculator calc(metrics);
    
    const auto& headers = calc.get_headers();
    BOOST_CHECK_EQUAL(headers[0], "receive_ts");
    BOOST_CHECK_EQUAL(headers[1], "mean");
    BOOST_CHECK_EQUAL(headers[2], "p99");
    BOOST_CHECK_EQUAL(headers[3], "median");
}

BOOST_AUTO_TEST_CASE(Test_NaN_Handling) {
    std::vector<std::string> metrics = {"mean", "std"};
    csv_median::StatsCalculator calc(metrics);
    
    // Только NaN значения
    calc.add_price(NAN);
    calc.add_price(INFINITY);
    
    auto stats = calc.get_stats();
    BOOST_CHECK(stats[0].has_value());  // mean должен работать
    BOOST_CHECK(stats[1].has_value());  // std тоже
}

BOOST_AUTO_TEST_SUITE_END()
