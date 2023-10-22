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
target_include_directories(RetroV PRIVATE "source" "include")
target_compile_features(RetroV PRIVATE cxx_std_23)
find_library(LIB_WINHVPLATFORM NAMES WinHvPlatform)
find_library(LIB_WINHVEMULATION NAMES WinHvEmulation)
find_library(LIB_DWMAPI NAMES dwmapi)
target_link_libraries(RetroV PRIVATE ${LIB_WINHVPLATFORM} ${LIB_WINHVEMULATION} ${LIB_DWMAPI} SDL2::SDL2 capstone::capstone)
set_target_properties(RetroV PROPERTIES LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'")
target_stack_size(RetroV "256MiB")


add_custom_command(
  OUTPUT  ${CMAKE_BINARY_DIR}/firmware/sys.bin
  OUTPUT  ${CMAKE_BINARY_DIR}/firmware/vid.bin
  COMMAND 
  COMMENT "Building ROM images ...")

# Custom target to invoke the custom command above
add_custom_target(
  FirmwareROMs    
  DEPENDS ${CMAKE_BINARY_DIR}/firmware/sys.bin 
          ${CMAKE_BINARY_DIR}/firmware/vid.bin)

# Ensure that our main target depends on the other build
add_dependencies(RetroV FirmwareROMs)

install (FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${CMAKE_INSTALL_PREFIX}/bin OPTIONAL)
install (TARGETS RetroV DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/extras/ROMS/ DESTINATION ${CMAKE_INSTALL_PREFIX}/ROMs/extras)
