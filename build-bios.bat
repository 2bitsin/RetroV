setlocal
call "C:\Devel\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
set CONFIG=Debug
cmake -B build -S . -DCMAKE_BUILD_TYPE=%CONFIG% -DCMAKE_INSTALL_PREFIX=workspace -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DVCPKG_APPLOCAL_DEPS=ON
cmake --build build --config %CONFIG% --target BsSystem.INT
cmake --build build --config %CONFIG% --target BsSystem.AMD
cmake --build build --config %CONFIG% --target BsVideo.INT
cmake --build build --config %CONFIG% --target BsVideo.AMD
cmake --install build --config %CONFIG%
endlocal