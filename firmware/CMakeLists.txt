project(RetroVFirmware LANGUAGES C CXX ASM)
cmake_minimum_required(VERSION 3.22)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}../source)
add_subdirectory(source)
