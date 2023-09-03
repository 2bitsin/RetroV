cmake_minimum_required(VERSION 3.22)
project(HvDOS)

set (CMAKE_SYSTEM_VERSION "10.0.22621.0")

# include some misc fucntionality
include(${CMAKE_SOURCE_DIR}/cmake/stack.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/fasm.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/msvcfix.cmake)

# find packages SDL2 and capstone
find_package(SDL2 CONFIG REQUIRED)
find_package(capstone CONFIG REQUIRED)

# Main HvDOS executable target
file(GLOB_RECURSE SOURCE_FILES source/*.cpp source/*.hpp)
add_executable(HvDOS ${SOURCE_FILES})
target_include_directories(HvDOS PRIVATE source)
target_compile_features(HvDOS PRIVATE cxx_std_23)
find_library(LIB_WINHVPLATFORM NAMES WinHvPlatform)
find_library(LIB_WINHVEMULATION NAMES WinHvEmulation)
target_link_libraries(HvDOS PRIVATE ${LIB_WINHVPLATFORM} ${LIB_WINHVEMULATION} SDL2::SDL2 capstone::capstone)
set_target_properties(HvDOS PROPERTIES LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'")
target_stack_size(HvDOS "256MiB")

# Use add_custom_command to compile bios/bios.asm to bios.bin using fasm
file(GLOB_RECURSE COM_BIOS_ASI source/bios/com/*.asi)
file(GLOB_RECURSE SYS_BIOS_ASI source/bios/sys/*.asi)
message (STATUS "SYS_BIOS_ASI: ${SYS_BIOS_ASI}")
add_fasm_target(BiosAMD "source/bios/sys/amd.asm" ${SYS_BIOS_ASI} ${COM_BIOS_ASI})
add_fasm_target(BiosIntel "source/bios/sys/intel.asm" ${SYS_BIOS_ASI} ${COM_BIOS_ASI})
file(GLOB_RECURSE VID_BIOS_ASI source/bios/vid/*.asi)
message (STATUS "VID_BIOS_ASI: ${VID_BIOS_ASI}")
add_fasm_target(Video "source/bios/vid/main.asm" ${VID_BIOS_ASI} ${COM_BIOS_ASI})

install (FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${CMAKE_INSTALL_PREFIX}/bin OPTIONAL)
install (TARGETS HvDOS DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/extras/ROMS DESTINATION ${CMAKE_INSTALL_PREFIX})