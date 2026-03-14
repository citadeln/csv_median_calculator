#!/bin/bash
set -euo pipefail

OUTDIR="./examples/input"
mkdir -p "$OUTDIR"

FILENAME="btcusdt_trade_200mb.csv"
BASE_TS=1716810808593627
TOTAL_ROWS=1000000  # 4 миллиона строк = ~230MB

echo "Генерация $FILENAME ($TOTAL_ROWS строк ≈ 530MB)..."

time awk -v base_ts="$BASE_TS" -v rows="$TOTAL_ROWS" '
BEGIN {
    print "receive_ts;exchange_ts;price;quantity;side"
    srand()
    
    for (i = 0; i < rows; i++) {
        ts = base_ts + i * 100
        ex_ts = ts - 2000
        
        # ЦЕЛАЯ ЧАСТЬ цены: 68479-68481 (случайно)
        base_price = 68479 + int(rand() * 3)  # 68479, 68480, 68481
        decimals = int(rand() * 1000000)      # 000000-999999
        
        rand_qty = int(rand() * 1000000)      # 0.000000-0.999999
        side = int(rand() * 2) ? "bid" : "ask"
        
        printf "%d;%d;%d.%06d;0.%06d;%s\n", 
               ts, ex_ts, base_price, decimals, rand_qty, side
    }
    printf "\n AWK завершён: %d строк\n", rows > "/dev/stderr"
}' > "$OUTDIR/$FILENAME"

# Результат
SIZE=$(du -sh "$OUTDIR/$FILENAME" | awk '{print $1}')
LINES=$(wc -l < "$OUTDIR/$FILENAME")
echo "Готово: $SIZE ($LINES строк)"
echo "Файл: $OUTDIR/$FILENAME"
