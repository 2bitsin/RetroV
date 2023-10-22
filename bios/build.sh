BUILD_DIR=${1:-'build'}
OUTPUT_DIR=${2:-'workspace/ROMs'}

cmake -B $BUILD_DIR -S . -G "Watcom WMake" \
  -DCMAKE_TOOLCHAIN_FILE=cmake/ow86.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$OUTPUT_DIR \
  -DCMAKE_VERBOSE_MAKEFILE=ON
cmake --build $BUILD_DIR --config Release -- -v
cmake --install $BUILD_DIR --config Release