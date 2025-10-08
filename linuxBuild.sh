cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j 3


cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cpack --config build/CPackConfig.cmake -G DEB
