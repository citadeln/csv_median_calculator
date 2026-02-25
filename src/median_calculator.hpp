#pragma once
#include <queue>
#include <vector>
#include <iomanip>
#include <sstream>
#include "csv_reader.hpp"

class RunningMedian {
private:
    using MaxHeap = std::priority_queue<double>;  // max-heap for lower half
    using MinHeap = std::priority_queue<double, std::vector<double>, std::greater<double>>;  // min-heap for upper half

    MaxHeap lower_;
    MinHeap upper_;
    double prev_median_ = 0.0;
    std::string prev_median_str_;

public:
    void add(double price) {
        // Add to lower heap, rebalance
        lower_.push(price);
        if (!upper_.empty() && lower_.top() > upper_.top()) {
            std::swap(lower_.top(), upper_.top());
            std::pop_heap(lower_); lower_.pop();
            std::push_heap(lower_);
            std::pop_heap(upper_); upper_.pop();
            std::push_heap(upper_);
        }
        // Balance sizes
        if (lower_.size() > upper_.size() + 1) {
            upper_.push(lower_.top());
            std::pop_heap(lower_); lower_.pop();
            std::push_heap(upper_);
        } else if (upper_.size() > lower_.size()) {
            lower_.push(upper_.top());
            std::pop_heap(upper_); upper_.pop();
            std::push_heap(lower_);
        }
    }

    bool median_changed(uint64_t ts, std::ostream& out) {
        double median;
        if (lower_.size() == upper_.size()) {
            median = (lower_.top() + upper_.top()) / 2.0;
        } else {
            median = lower_.top();
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(8) << median;
        std::string curr_str = oss.str();

        if (curr_str != prev_median_str_) {
            out << ts << ";" << curr_str << "\n";
            prev_median_ = median;
            prev_median_str_ = curr_str;
            return true;
        }
        return false;
    }
};
