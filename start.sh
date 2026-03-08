#!/bin/bash
# =============================================================================
# start.sh - Быстрая сборка и запуск csv_median_calculator v2.0 (C++23)
# Автор: Anastasiya Dorohina
# =============================================================================

set -e  # Остановка при любой ошибке

echo "csv_median_calculator v2.0 - Сборка и запуск..."

# 1. Очистка предыдущей сборки
echo "Очистка build директории..."
rm -rf build/ compile_commands.json

# 2. Конфигурация CMake + LSP поддержка (VSCode/Neovim)
echo "CMake конфигурация..."
cmake -B build \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_BUILD_TYPE=Release

# 3. Копирование compile_commands.json для IDE (автодополнение, рефакторинг)
echo "LSP: копируем compile_commands.json..."
cp build/compile_commands.json .

# 4. Параллельная сборка
# Автоопределение ядер CPU для сборки
CPU_COUNT=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)
echo "Сборка проекта ($CPU_COUNT ядер)..."
cmake --build build/ -j"$CPU_COUNT"


# 5. Запуск приложения с конфигурацией
echo "Запуск csv_median_calculator..."
./build/csv_median_calculator --config config.toml

# 6. Показываем результат (первые 10 строк)
echo "Результат median_result.csv:"
echo "----------------------------------------"
if [ -f "output/median_result.csv" ]; then
    head -10 output/median_result.csv
else
    echo "median_result.csv не найден!"
    exit 1
fi
