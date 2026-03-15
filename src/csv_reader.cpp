/**
 * \file csv_reader.cpp
 * \author Anastasiya Dorohina
 * \brief CSV парсер + рекурсивный поиск файлов по маске (ТЗ 3.1)
 * \date 2026-03-15
 * \version 2.2
 */

#include "csv_reader.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <algorithm>
#include <ranges>

namespace csv_median {
namespace fs = std::filesystem;

/**
 * \brief Парсит CSV файл в MarketEvent структуру
 * 
 * **Алгоритм парсинга (ТЗ 3.1, 4.4):**
 * 1. Открывает файл, возвращает {} при ошибке открытия
 * 2. **Валидация заголовка:** ищет "receive_ts" в первой строке
 *    - Отсутствует → **весь файл исключается** с ошибкой (ТЗ)
 * 3. Парсит каждую строку: `receive_ts;exchange_ts;price;...`
 * 4. Извлекает **1-ю колонку** (receive_ts → uint64_t) и **3-ю колонку** (price → double)
 * 5. **try-catch** игнорирует некорректные строки (robustness ТЗ)
 * 
 * \param[in] file Путь к CSV файлу (.csv)
 * \return Вектор MarketEvent или {} при неверном заголовке
 * \warning Файл с неверным заголовком **полностью исключается**
 * \Complexity O(N) где N — количество строк в файле
 */
std::vector<MarketEvent> parse_csv(const fs::path& file) {
    std::vector<MarketEvent> events;
    std::ifstream csv_file(file);
    
    if (!csv_file.is_open()) {
        spdlog::warn("Cannot open CSV: {}", file.string());
        return events;
    }
    
    std::string line;
    bool first_line = true;
    
    while (std::getline(csv_file, line)) {
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
        
        // Поиск разделителей ';' для извлечения колонок 1 и 3
        std::string_view line_view{line};
        auto pos1 = line_view.find(';');           // receive_ts конец
        auto pos2 = line_view.find(';', pos1 + 1); // exchange_ts конец
        auto pos3 = line_view.find(';', pos2 + 1); // price конец
        
        if (pos1 == std::string_view::npos || 
            pos2 == std::string_view::npos || 
            pos3 == std::string_view::npos) {
            continue;
        }
        
        try {
            // Zero-copy парсинг: string_view.data() → stoull/stod
            std::string_view ts_view = line_view.substr(0, pos1);
            std::string_view price_view = line_view.substr(pos2 + 1, pos3 - pos2 - 1);
            
            uint64_t ts = std::stoull(ts_view.data(), nullptr, 10);
            double price = std::stod(price_view.data(), nullptr);
            
            events.emplace_back(MarketEvent{ts, price});
        } catch (...) {
            // ТЗ 4.4: некорректные строки игнорируются
        }
    }
    
    spdlog::debug("Parsed {} events from {}", events.size(), file.filename().string());
    return events;
}

/**
 * \brief Рекурсивно собирает CSV файлы по filename_mask и парсит их
 * 
 * **Полный алгоритм ТЗ 3.1:**
 * 1. **recursive_directory_iterator** → сканирует input_dir рекурсивно
 * 2. Фильтрует **только .csv** регулярные файлы
 * 3. **Фильтрация по маске:** filename_mask = ['level','trade']
 *    - Пустой список → все CSV файлы
 *    - Иначе → файл содержит **хотя бы одну** маску как подстроку
 * 4. **std::ranges::sort** → лексикографическая сортировка файлов
 * 5. **Последовательный парсинг** parse_csv() **в порядке сортировки**
 * 6. **make_move_iterator** → zero-copy конкатенация всех событий
 * 
 * **Детерминизм (ТЗ 3.1):** 
 * - Лексикографический порядок файлов гарантирует воспроизводимость
 * - stable_sort(events) в main.cpp сохраняет относительный порядок
 * 
 * **Пример:**
 * ```
 * input_dir/
 * ├── level_20240315.csv     → ✓ matches 'level'
 * ├── trade_20240315.csv     → ✓ matches 'trade'  
 * ├── other.txt              → ✗ не .csv
 * └── subdir/other.csv       → ✗ не содержит 'level'/'trade'
 * ```
 * 
 * \param[in] input_dir Директория для рекурсивного поиска CSV
 * \param[in] filename_mask Список строк-масок (пустой = все CSV)
 * \return Объединённый поток MarketEvent из всех подходящих файлов
 * \Complexity O(F log F + ΣN_f) где F — файлы, N_f — строк в файле f
 */
std::vector<MarketEvent> read_csv_files(const fs::path& input_dir, const std::vector<std::string>& filename_mask) {
    std::vector<MarketEvent> all_events;
    
    if (!fs::exists(input_dir)) {
        spdlog::warn("Input dir not found: {}", input_dir.string());
        return all_events;
    }
    
    spdlog::debug("Scanning directory: {}", input_dir.string());
    
    std::vector<fs::path> csv_files;
    for (const auto& entry : fs::recursive_directory_iterator(input_dir)) {
        // Фильтр: только CSV регулярные файлы
        if (!entry.is_regular_file() || entry.path().extension() != ".csv") 
            continue;
        
        // Извлечение имени файла для фильтрации по маске
        std::string filename = entry.path().filename().string();
        spdlog::debug("Found CSV: {}", filename);
        
        // Проверка соответствия filename_mask
        bool matches_mask = filename_mask.empty();
        for (const std::string& mask : filename_mask) {
            if (filename.find(mask) != std::string::npos) {
                matches_mask = true;
                spdlog::debug("  → matches mask '{}'", mask);
                break;
            }
        }
        
        if (matches_mask) {
            csv_files.push_back(entry.path());
            spdlog::info("Added CSV file: {}", filename);
        }
    }
    
    // Лексикографическая сортировка файлов (ТЗ 3.1 детерминизм)
    std::ranges::sort(csv_files);
    spdlog::info("Found {} CSV files matching mask", csv_files.size());
    
    // Последовательная обработка в порядке сортировки + zero-copy
    for (auto&& file : csv_files) {
        auto events = parse_csv(file);
        if (!events.empty()) {
            all_events.insert(all_events.end(),
                            std::make_move_iterator(events.begin()),
                            std::make_move_iterator(events.end()));
        }
    }
    
    return all_events;
}

} // namespace csv_median
