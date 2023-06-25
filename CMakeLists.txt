cmake_minimum_required(VERSION 3.22)
project(HvDOS)

file(GLOB_RECURSE SOURCE_FILES source/*.cpp source/*.hpp)

add_executable(HvDOS ${SOURCE_FILES})
target_include_directories(HvDOS PRIVATE source)
target_compile_features(HvDOS PRIVATE cxx_std_23)

# Use add_custom_command to compile bios/bios.asm to bios.bin using fasm
find_program(FASM_COMMAND fasm)
set(BIOS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/bios/bios.asm)
set(BIOS_TARGET BIOS.BIN)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${BIOS_TARGET}
  COMMAND ${FASM_COMMAND} ${BIOS_SOURCE} ${CMAKE_CURRENT_BINARY_DIR}/${BIOS_TARGET}
  DEPENDS ${BIOS_SOURCE}
  COMMENT "Compiling bios.asm to ${BIOS_TARGET}"  
)
add_custom_target(bios_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${BIOS_TARGET})

find_library(LIB_WINHVPLATFORM NAMES WinHvPlatform)
target_link_libraries(HvDOS PRIVATE ${LIB_WINHVPLATFORM})

set (STACK_SIZE 268435456)
if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${STACK_SIZE}")
else()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,${STACK_SIZE}")
endif()


install (TARGETS HvDOS DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIOS_TARGET} DESTINATION ${CMAKE_INSTALL_PREFIX}/roms)

message(WARNING ${SOURCE_FILES})
