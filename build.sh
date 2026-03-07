rm -rf build/ compile_commands.json
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cp build/compile_commands.json .
cmake --build build/ -j8
