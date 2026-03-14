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
echo -e "\nСборка проекта ($CPU_COUNT ядер)..."
cmake --build build/ -j"$CPU_COUNT"
