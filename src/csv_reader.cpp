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

/**
 * \brief Парсит CSV файл в MarketEvent события
 * 
 * **Алгоритм парсинга (ТЗ):**
 * - Чтение строк через std::getline()
 * - Проверка заголовка: обязателен "receive_ts" (ТЗ: без него файл исключается)
 * - Поиск позиций ';' для извлечения receive_ts (1-й столбец) и price (3-й столбец)
 * - Поддержка level.csv (6 колонок) и trade.csv (5 колонок) через одинаковый парсинг первых 3 полей
 * - Игнорирование некорректных строк (try-catch для std::stoull/stod)
 * - **Весь файл исключается при неверном заголовке** (ТЗ: источник данных невалиден)
 * 
 * \param file Путь к CSV файлу
 * \return Список MarketEvent или пустой вектор при ошибке/невалидном формате
 * \warning Некорректный header → немедленный возврат {} (ТЗ)
 */
std::vector<MarketEvent> parse_csv(const fs::path& file) {
    std::vector<MarketEvent> events;
    std::ifstream csv_file(file);
    
    // Файл не открылся → ТЗ: пустой результат без crash
    if (!csv_file.is_open()) {
        spdlog::warn("Cannot open CSV: {}", file.string());
        return events;
    }
    
    std::string line;
    bool first_line = true;
    
    while (std::getline(csv_file, line)) {
        // Обработка заголовка: ТЗ требует "receive_ts"
        if (first_line) {
            first_line = false;
            if (line.find("receive_ts") == std::string::npos) {
                spdlog::error("Invalid CSV header in {}: {}", 
                             file.filename().string(), 
                             line.substr(0, 50));
                return {};  // ТЗ: весь файл исключается
            }
            continue;
        }
        
        // Поиск разделителей для извлечения receive_ts и price
        size_t semicolon1 = line.find(';');           // receive_ts конец
        size_t semicolon2 = line.find(';', semicolon1 + 1);  // exchange_ts конец  
        size_t semicolon3 = line.find(';', semicolon2 + 1);  // price конец
        
        // Минимум 3 колонки для валидности (ТЗ robustness)
        if (semicolon1 == std::string::npos || 
            semicolon2 == std::string::npos || 
            semicolon3 == std::string::npos) {
            continue;
        }
        
        try {
            // Парсинг чисел с обработкой ошибок
            uint64_t ts = std::stoull(line.substr(0, semicolon1));
            double price = std::stod(line.substr(semicolon2 + 1, semicolon3 - semicolon2 - 1));
            events.emplace_back(MarketEvent{ts, price});
        } catch (...) {
            // ТЗ: некорректные строки игнорируются (не crash)
        }
    }
    
    spdlog::debug("Parsed {} events from {}", events.size(), file.filename().string());
    return events;
}

/**
 * \brief Сканирует директорию и собирает CSV по filename_mask
 * 
 * **Полный алгоритм (строго по ТЗ):**
 * 1. recursive_directory_iterator(input_dir) → все файлы рекурсивно
 * 2. Фильтр: только .csv файлы
 * 3. filename_mask: **поиск подстроки** в имени файла (или все CSV при пустой маске)
 * 4. **Лексикографическая сортировка** найденных файлов (std::sort)
 * 5. Последовательный парсинг parse_csv() **в порядке сортировки**
 * 6. Конкатенация всех событий в единый поток
 * 
 * **Детерминизм:** stable_sort(events) + лексикографическая сортировка файлов
 * гарантирует воспроизводимый результат при одинаковых данных.
 * 
 * \param input_dir Директория поиска (может не существовать → пустой результат)
 * \param filename_mask Список строк-подмасок (пустой = все CSV)
 * \return Все MarketEvent из подходящих файлов в порядке обработки
 */
std::vector<MarketEvent> read_csv_files(const fs::path& input_dir, const std::vector<std::string>& filename_mask) {
    std::vector<MarketEvent> all_events;
    
    // ТЗ: отсутствующая директория → graceful пустой результат
    if (!fs::exists(input_dir)) {
        spdlog::warn("Input dir not found: {}", input_dir.string());
        return all_events;
    }
    
    // Сбор CSV файлов по маске (ТЗ: substring search)
    std::vector<fs::path> csv_files;
    for (const auto& entry : fs::recursive_directory_iterator(input_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".csv") {
            std::string filename = entry.path().filename().string();
            bool matches_mask = filename_mask.empty(); // Пустая маска = все CSV
            
            // Проверка любой маски (ТЗ: OR логика)
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
    
    // ТЗ: лексикографическая сортировка файлов перед обработкой
    std::sort(csv_files.begin(), csv_files.end());
    spdlog::info("Found {} CSV files matching mask", csv_files.size());
    
    // Последовательная обработка в **лексикографическом порядке**
    for (const auto& file : csv_files) {
        auto events = parse_csv(file);
        if (!events.empty()) {
            all_events.insert(all_events.end(), events.begin(), events.end());
        }
    }
    
    return all_events;
}

} // namespace csv_median
