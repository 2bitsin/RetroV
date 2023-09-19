cmake_minimum_required(VERSION 3.22)
project(RetroV)

set (CMAKE_SYSTEM_VERSION "10.0.22621.0")

# include some misc fucntionality
include(${CMAKE_SOURCE_DIR}/cmake/stack.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/fasm.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/msvcfix.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/utils.cmake)

# find packages SDL2 and capstone
find_package(SDL2 CONFIG REQUIRED)
find_package(capstone CONFIG REQUIRED)

# Main RetroV executable target
file(GLOB_RECURSE SOURCE_FILES source/*.cpp source/*.hpp)
add_executable(RetroV ${SOURCE_FILES})
target_include_directories(RetroV PRIVATE source)
target_compile_features(RetroV PRIVATE cxx_std_23)
find_library(LIB_WINHVPLATFORM NAMES WinHvPlatform)
find_library(LIB_WINHVEMULATION NAMES WinHvEmulation)
find_library(LIB_DWMAPI NAMES dwmapi)
target_link_libraries(RetroV PRIVATE ${LIB_WINHVPLATFORM} ${LIB_WINHVEMULATION} ${LIB_DWMAPI} SDL2::SDL2 capstone::capstone)
set_target_properties(RetroV PROPERTIES LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'")
target_stack_size(RetroV "256MiB")

string(TIMESTAMP TODAY "%Y-%m-%d")
string(TIMESTAMP NOW "%H:%M:%S")
preprocess_file(hypercall_ash source/bios/com/hypercall.ash source/bios/gen/hypercall.asi)
# Use add_custom_command to compile bios/bios.asm to bios.bin using fasm
file(GLOB_RECURSE COM_BIOS_ASI source/bios/com/*.asi)
file(GLOB_RECURSE SYS_BIOS_ASI source/bios/sys/*.asi)
file(GLOB_RECURSE VID_BIOS_ASI source/bios/vid/*.asi)
list(APPEND COM_BIOS_ASI source/bios/gen/hypercall.asi)

message (STATUS "SYS_BIOS_ASI: ${SYS_BIOS_ASI}")
message (STATUS "VID_BIOS_ASI: ${VID_BIOS_ASI}")

add_fasm_target(BsSystem.AMD "source/bios/sys/main.asm" ${SYS_BIOS_ASI} ${COM_BIOS_ASI})
add_fasm_option(BsSystem.AMD -d hypercall=vmmcall -d date=${TODAY} -d time=${NOW})
add_fasm_target(BsSystem.INT "source/bios/sys/main.asm" ${SYS_BIOS_ASI} ${COM_BIOS_ASI})
add_fasm_option(BsSystem.INT -d hypercall=vmcall -d date=${TODAY} -d time=${NOW})

add_fasm_target(BsVideo.AMD "source/bios/vid/main.asm" ${VID_BIOS_ASI} ${COM_BIOS_ASI})
add_fasm_option(BsVideo.AMD -d hypercall=vmmcall -d date=${TODAY} -d time=${NOW})
add_fasm_target(BsVideo.INT "source/bios/vid/main.asm" ${VID_BIOS_ASI} ${COM_BIOS_ASI})
add_fasm_option(BsVideo.INT -d hypercall=vmcall -d date=${TODAY} -d time=${NOW})

install (FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${CMAKE_INSTALL_PREFIX}/bin OPTIONAL)
install (TARGETS RetroV DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/extras/ROMS/ DESTINATION ${CMAKE_INSTALL_PREFIX}/ROMs/extras)
install (FILES $<TARGET_PROPERTY:BsVideo.AMD,OUTPUT_FILE_PATH> DESTINATION ${CMAKE_INSTALL_PREFIX}/ROMS/AMD RENAME video.bin) 
install (FILES $<TARGET_PROPERTY:BsVideo.INT,OUTPUT_FILE_PATH> DESTINATION ${CMAKE_INSTALL_PREFIX}/ROMS/Intel RENAME video.bin)
install (FILES $<TARGET_PROPERTY:BsSystem.AMD,OUTPUT_FILE_PATH> DESTINATION ${CMAKE_INSTALL_PREFIX}/ROMS/AMD RENAME system.bin)
install (FILES $<TARGET_PROPERTY:BsSystem.INT,OUTPUT_FILE_PATH> DESTINATION ${CMAKE_INSTALL_PREFIX}/ROMS/Intel RENAME system.bin)
