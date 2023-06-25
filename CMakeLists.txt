cmake_minimum_required(VERSION 3.22)
project(HvDOS)

include(${CMAKE_SOURCE_DIR}/cmake/stack.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/fasm.cmake)

# Main HvDOS executable target
file(GLOB_RECURSE SOURCE_FILES source/*.cpp source/*.hpp)
add_executable(HvDOS ${SOURCE_FILES})
target_include_directories(HvDOS PRIVATE source)
target_compile_features(HvDOS PRIVATE cxx_std_23)
find_library(LIB_WINHVPLATFORM NAMES WinHvPlatform)
target_link_libraries(HvDOS PRIVATE ${LIB_WINHVPLATFORM})
set_target_stack_size(HvDOS "256MiB")

# Use add_custom_command to compile bios/bios.asm to bios.bin using fasm
file(GLOB_RECURSE SOURCE_FILES_ASI bios/*.asi)
add_fasm_target(BiosAMD   
  "source/bios/amd.asm"  
  ${SOURCE_FILES_ASI})
add_fasm_target(BiosIntel 
  "source/bios/intel.asm" 
  ${SOURCE_FILES_ASI})

install (TARGETS HvDOS DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

message(WARNING ${SOURCE_FILES})
