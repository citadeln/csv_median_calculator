# Запуск приложения с конфигурацией
echo "Запуск csv_median_calculator..."
./build/csv_median_calculator --config config.toml

# 6. Показываем результат (первые 10 строк)
echo -e "\nРезультат median_result.csv (первые 10 строк):"
echo "----------------------------------------"
if [ -f "examples/output/median_result.csv" ]; then
    head -10 examples/output/median_result.csv
else
    echo "median_result.csv не найден!"
    exit 1
fi
