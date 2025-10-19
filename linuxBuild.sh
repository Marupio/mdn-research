
# Build on linux
#in build folder, clear / mkdir as required
mkdir build
cd build

# actual build, from <project>/build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DLogs=OFF ..
cmake --build build --config Release -j 4
cpack --config build/CPackConfig.cmake -G DEB

# Getting hash
Get-FileHash .\build\dist\MDN-<ver>-win64-Release.zip -Algorithm SHA256 | Format-List
sha256sum mdn_1.1.0-1_amd64.deb

# Old build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j 3
