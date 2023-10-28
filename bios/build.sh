#!/bin/bash
set -e

BUILD_DIR=${1:-'/base/build/ROMs'}
OUTPUT_DIR=${2:-'/base/workspace/ROMs'}
SCRIPT_DIR="$(dirname $(readlink -f $0))"
TOOLCHAIN_FILE=$(readlink -f $SCRIPT_DIR/cmake/ow86.cmake)

echo BUILD_DIR=\"$BUILD_DIR\"
echo OUTPUT_DIR=\"$OUTPUT_DIR\"
echo SCRIPT_DIR=\"$SCRIPT_DIR\"
echo TOOLCHAIN_FILE=\"$TOOLCHAIN_FILE\"

rm -rf $BUILD_DIR
pushd $SCRIPT_DIR
cmake -G "Watcom WMake"                  \
  -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
  -DCMAKE_INSTALL_PREFIX=$OUTPUT_DIR     \
  -DCMAKE_BUILD_TYPE=Release             \
  -DCMAKE_VERBOSE_MAKEFILE=ON            \
  -B $BUILD_DIR                          \
  -S $SCRIPT_DIR
cmake --build $BUILD_DIR --config Release -- -v
cmake --install $BUILD_DIR --config Release
popd