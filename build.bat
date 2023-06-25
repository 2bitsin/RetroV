call "C:\Devel\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
set CONFIG=Debug
cmake -B build -S . -DCMAKE_BUILD_TYPE=%CONFIG% -DCMAKE_INSTALL_PREFIX=workspace
cmake --build build --config %CONFIG%
cmake --install build --config %CONFIG%
