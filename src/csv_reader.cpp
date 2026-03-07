#include "csv_reader.hpp"
#include <spdlog/spdlog.h>
#include <ranges>
#include <fstream>
#include <algorithm>

namespace csv_median {
namespace fs = std::filesystem;
namespace views = std::views;

std::vector<MarketEvent> parse_csv(const fs::path& file) {
    std::vector<MarketEvent> events;
    std::ifstream csv_file(file);
    
    if (!csv_file.is_open()) {
        spdlog::warn("📄 Cannot open CSV: {}", file.string());
        return events;
    }
    
    std::string line;
    bool first_line = true;
    
    while (std::getline(csv_file, line)) {
        if (first_line) {
            first_line = false;
            // Проверяем заголовок (упрощенно)
            if (line.find("receive_ts") == std::string::npos) {
                spdlog::error("❌ Invalid CSV header in {}: {}", file.filename().string(), line.substr(0, 50));
                return {}; // Файл целиком исключаем (ТЗ)
            }
            continue;
        }
        
        // Парсим строку: receive_ts;price;...
        size_t semicolon1 = line.find(';');
        size_t semicolon2 = line.find(';', semicolon1 + 1);
        
        if (semicolon1 == std::string::npos || semicolon2 == std::string::npos) {
            continue; // Некорректная строка
        }
        
        try {
            uint64_t ts = std::stoull(line.substr(0, semicolon1));
            double price = std::stod(line.substr(semicolon1 + 1, semicolon2 - semicolon1 - 1));
            events.emplace_back(MarketEvent{ts, price});
        } catch (...) {
            // Игнорируем некорректные числа
        }
    }
    
    spdlog::debug("📊 Parsed {} events from {}", events.size(), file.filename().string());
    return events;
}

std::vector<MarketEvent> read_csv_files(const fs::path& input_dir, const std::vector<std::string>& filename_mask) {
    std::vector<MarketEvent> all_events;
    
    if (!fs::exists(input_dir)) {
        spdlog::warn("📁 Input dir not found: {}", input_dir.string());
        return all_events;
    }
    
    // 1. Собираем CSV файлы по маске (ТЗ)
    std::vector<fs::path> csv_files;
    for (const auto& entry : fs::recursive_directory_iterator(input_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".csv") {
            std::string filename = entry.path().filename().string();
            bool matches_mask = filename_mask.empty();
            
            // Проверяем filename_mask (поиск подстроки, ТЗ)
            for (const auto& mask : filename_mask) {
                if (filename.find(mask) != std::string::npos) {
                    matches_mask = true;
                    break;
                }
            }
            
            if (matches_mask) {
                csv_files.push_back(entry.path());
            }
        }
    }
    
    // 2. Лексикографическая сортировка файлов (ТЗ)
    std::sort(csv_files.begin(), csv_files.end());
    
    spdlog::info("🔍 Found {} CSV files matching mask", csv_files.size());
    
    // 3. Читаем файлы в лексикографическом порядке
    for (const auto& file : csv_files) {
        auto events = parse_csv(file);
        if (!events.empty()) {
            all_events.insert(all_events.end(), events.begin(), events.end());
        }
    }
    
    return all_events;
}

} // namespace csv_median
