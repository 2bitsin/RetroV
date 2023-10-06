rm -rf build/*
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./cmake/Ia16FreeStanding.cmake -DCMAKE_INSTALL_PREFIX=/var/install
cmake --build build --config Release
cmake --install build --config Release 
