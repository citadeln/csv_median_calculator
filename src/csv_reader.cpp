/**
 * \file csv_reader.cpp
 * \author Anastasiya Dorohina
 * \brief CSV парсер + фильтр файлов по маске
 * \date 2026-03-08
 * \version 2.0
 */

#include "csv_reader.hpp"
#include <spdlog/spdlog.h>
#include <ranges>
#include <fstream>
#include <algorithm>

namespace csv_median {
namespace fs = std::filesystem;
namespace views = std::views;

/**
 * \brief Парсит CSV файл в MarketEvent события
 * 
 * Формат CSV: receive_ts;price;volume;symbol
 * Алгоритм:
 *  - Проверка заголовка (обязателен "receive_ts")
 *  - Поиск 1-го и 2-го ';' для извлечения ts+price  
 *  - Пропуск некорректных строк
 * 
 * \param file Путь к CSV файлу
 * \return События или пустой вектор при ошибке
 * \warning Весь файл исключается при неверном заголовке (ТЗ)
 */
std::vector<MarketEvent> parse_csv(const fs::path& file) {
    std::vector<MarketEvent> events;
    std::ifstream csv_file(file);
    
    // Файл не открылся
    if (!csv_file.is_open()) {
        spdlog::warn("📄 Cannot open CSV: {}", file.string());
        return events;
    }
    
    std::string line;
    bool first_line = true;
    
    while (std::getline(csv_file, line)) {
        if (first_line) {
            first_line = false;
            // ТЗ: без "receive_ts" в header — файл исключаем
            if (line.find("receive_ts") == std::string::npos) {
                spdlog::error("❌ Invalid CSV header in {}: {}", 
                             file.filename().string(), 
                             line.substr(0, 50));
                return {};
            }
            continue;
        }
        
        // Извлекаем receive_ts и price через позиции ';'
        size_t semicolon1 = line.find(';');
        size_t semicolon2 = line.find(';', semicolon1 + 1);
        
        // Некорректный формат строки
        if (semicolon1 == std::string::npos || semicolon2 == std::string::npos) {
            continue;
        }
        
        try {
            uint64_t ts = std::stoull(line.substr(0, semicolon1));
            double price = std::stod(line.substr(semicolon1 + 1, semicolon2 - semicolon1 - 1));
            events.emplace_back(MarketEvent{ts, price});
        } catch (...) {
            // Игнорируем некорректные числа (robustness)
        }
    }
    
    spdlog::debug("📊 Parsed {} events from {}", events.size(), file.filename().string());
    return events;
}

/**
 * \brief Сканирует директорию и собирает CSV по filename_mask
 * 
 * Алгоритм (ТЗ):
 *  - recursive_directory_iterator + фильтр .csv
 *  - filename_mask: подстрока в имени (или все файлы)
 *  - Лексикографическая сортировка файлов
 *  - Последовательный парсинг parse_csv()
 * 
 * \param input_dir Директория поиска
 * \param filename_mask Фильтры имен файлов
 * \return Все MarketEvent из подходящих файлов
 */
std::vector<MarketEvent> read_csv_files(const fs::path& input_dir, const std::vector<std::string>& filename_mask) {
    std::vector<MarketEvent> all_events;
    
    // Директория не существует
    if (!fs::exists(input_dir)) {
        spdlog::warn("📁 Input dir not found: {}", input_dir.string());
        return all_events;
    }
    
    // Собираем CSV файлы по маске
    std::vector<fs::path> csv_files;
    for (const auto& entry : fs::recursive_directory_iterator(input_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".csv") {
            std::string filename = entry.path().filename().string();
            bool matches_mask = filename_mask.empty(); // Пустая маска = все CSV
            
            // Проверяем совпадение с любой маской
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
    
    // Лексикографическая сортировка файлов
    std::sort(csv_files.begin(), csv_files.end());
    
    spdlog::info("🔍 Found {} CSV files matching mask", csv_files.size());
    
    // Последовательное чтение файлов
    for (const auto& file : csv_files) {
        auto events = parse_csv(file);
        if (!events.empty()) {
            all_events.insert(all_events.end(), events.begin(), events.end());
        }
    }
    
    return all_events;
}

} // namespace csv_median
