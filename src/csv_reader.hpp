/**
 * \file csv_reader.hpp
 * \author Anastasiya Dorohina
 * \brief CSV парсер + файловый фильтр
 * \date 2026-03-08
 * \version 2.0
 */

#pragma once

#include "market_record.hpp"
#include <filesystem>
#include <vector>
#include <string>

namespace csv_median {
namespace fs = std::filesystem;

/**
 * \brief Парсит CSV файл в MarketEvent
 * \param file Путь к CSV файлу
 * \return Список событий или пустой вектор при ошибке
 */
[[nodiscard]] std::vector<MarketEvent> parse_csv(const fs::path& file);

/**
 * \brief Сканирует директорию, фильтрует CSV по маске
 * \param input_dir Директория поиска
 * \param filename_mask Фильтры имен файлов
 * \return Все MarketEvent в лексикографическом порядке файлов
 */
[[nodiscard]] std::vector<MarketEvent> read_csv_files(
    const fs::path& input_dir,
    const std::vector<std::string>& filename_mask);

} // namespace csv_median
