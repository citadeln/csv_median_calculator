/// \file median_calculator.hpp
/// \brief Hybrid median: nth_element (exact) + P² (streaming O(1))
#pragma once

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/psquare_quantile.hpp>
#include <algorithm>
#include <vector>
#include <optional>
#include <mutex>

namespace median {
using namespace boost::accumulators;

constexpr size_t SEED_THRESHOLD = 64;  ///< nth_element до 64, потом P²

class calculator {
public:
    explicit calculator() noexcept = default;

    /// \brief Add price incrementally
    void add_price(double price_) noexcept {
        std::lock_guard lock(mutex_);
        if (buffer_.size() < SEED_THRESHOLD) {
            buffer_.push_back(price_);
        } else {
            switch_to_psquare(price_);
        }
    }

    /// \brief Get current median (8 decimals)
    [[nodiscard]] std::optional<double> median() const noexcept {
        std::lock_guard lock(mutex_);
        if (buffer_.empty() && !psquare_acc_) return std::nullopt;
        if (psquare_acc_) {
            return quantile(*psquare_acc_, quantile_probability = 0.5);
        }
        return exact_median_from_buffer();
    }

private:
    void switch_to_psquare(double price_) noexcept {
        if (!psquare_acc_) {
            psquare_acc_ = std::make_unique<accumulator_set<double, features<psquare_quantile>>>();
            for (double v : buffer_) {
                (*psquare_acc_)(v);
            }
            buffer_.clear();
        }
        (*psquare_acc_)(price_);
    }

    [[nodiscard]] double exact_median_from_buffer() const noexcept {
        if (buffer_.empty()) return 0.0;
        std::vector tmp = buffer_;
        size_t n = tmp.size();
        size_t mid = n / 2;
        std::nth_element(tmp.begin(), tmp.begin() + mid, tmp.end());
        if (n % 2 == 1) return tmp[mid];
        std::nth_element(tmp.begin(), tmp.begin() + mid - 1, tmp.begin() + mid);
        return (tmp[mid - 1] + tmp[mid]) / 2.0;
    }

    mutable std::mutex mutex_;
    std::vector<double> buffer_;
    std::unique_ptr<accumulator_set<double, features<psquare_quantile>>> psquare_acc_;
};
}  // namespace median
