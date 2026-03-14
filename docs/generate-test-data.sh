#!/bin/bash
set -euo pipefail

# Устанавливаем директорию для хранения тестовых данных
OUTDIR="./examples/input"
mkdir -p "$OUTDIR"

# Генерируем тестовый CSV размером ~200 MB (~1.5 млн строк)
echo "🎲 Генерация 200MB test CSV..."
BASE_TS=1716810808593627
ROWS=1500000  # Количество строк
COUNTER=0

while (( COUNTER < ROWS )); do
    TS=$(( BASE_TS + COUNTER * 100 ))
    PRICE=$(printf "%.8f" $(bc <<< "scale=8; 68480 + ($RANDOM % 100)/100"))
    QTY=$(printf "%.6f" $(bc <<< "scale=6; 0 + ($RANDOM % 100)/100"))
    SIDE=$(( RANDOM % 2 )) && SIDE="bid" || SIDE="ask"
    echo "$TS;${TS}-2000;$PRICE;$QTY;$SIDE"
    (( COUNTER++ ))
done > "$OUTDIR/btcusdt_trade_200mb.csv"

# Проверяем размер файла
SIZE=$(du -sh "$OUTDIR/btcusdt_trade_200mb.csv" | awk '{print $1}')
echo "Размер файла: $SIZE"

# Запускаем бенчмарк
echo "Время выполнения теста:"
time ./build/csv_median_calculator --config config.toml
