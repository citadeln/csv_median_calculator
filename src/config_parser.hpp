/**
 * \file config_parser.hpp
 * \author Anastasiya Dorohina
 * \brief TOML конфигурация проекта
 * \date 2026-03-08
 * \version 2.0
 */

#pragma once
#include <filesystem>
#include <vector>
#include <string>

namespace csv_median {
namespace fs = std::filesystem;

/**
 * \struct Config
 * \brief Конфигурация обработки CSV файлов
 */
struct Config {
    fs::path              input_dir;      ///< Директория с CSV файлами
    fs::path              output_dir;     ///< Директория результатов
    std::vector<std::string> filename_mask; ///< Фильтры имен файлов (*.csv)
};

/**
 * \brief Парсит config.toml в Config структуру
 * \param path Путь к config.toml файлу
 * \return Config с ТЗ дефолтами при ошибке
 * \warning При отсутствии файла возвращает дефолты ТЗ
 */
Config parse_config(const fs::path& path);

} // namespace csv_median
