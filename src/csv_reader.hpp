#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace fs = std::filesystem;

struct MarketRecord {
    uint64_t receive_ts = 0;
    double price = 0.0;

    bool operator<(const MarketRecord& other) const noexcept {
        return receive_ts < other.receive_ts;
    }
    bool operator>(const MarketRecord& other) const noexcept {
        return receive_ts > other.receive_ts;
    }
};

inline std::vector<MarketRecord> read_csv_file(const fs::path& file_path) {
    std::vector<MarketRecord> records;

    std::ifstream file(file_path);
    if (!file.is_open()) {
        spdlog::warn("Cannot open file: {}", file_path.string());
        return {};
    }

    std::string line, header;
    std::getline(file, header);  // Skip header

    // Validate header: must have receive_ts and price
    if (header.find("receive_ts") == std::string::npos || header.find("price") == std::string::npos) {
        spdlog::error("Invalid header in {}: missing receive_ts or price", file_path.string());
        return {};
    }

    size_t line_num = 1;
    while (std::getline(file, line)) {
        ++line_num;
        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> fields;
        while (std::getline(iss, token, ';')) {
            fields.push_back(token);
        }
        if (fields.size() < 2) continue;  // Skip invalid lines

        try {
            MarketRecord rec;
            rec.receive_ts = std::stoull(fields[0]);
            rec.price = std::stod(fields[2]);  // price is 3rd field (0:receive_ts,1:exchange_ts,2:price)
            records.push_back(rec);
        } catch (const std::exception& e) {
            spdlog::warn("Invalid data at line {} in {}: {}", line_num, file_path.string(), e.what());
            continue;
        }
    }

    spdlog::info("Read {} records from {}", records.size(), file_path.string());
    return records;
}
