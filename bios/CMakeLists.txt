cmake_minimum_required(VERSION 3.22)

project(ReVBios C ASM CXX)

set (CMAKE_EXE_LINKER_FLAGS "")

set (LINK_LIBRARIES ${WATCOM_DIR}/lib286/dos/clibs.lib)

set (SYS_IMAGE_NAME "revbios.sys")
set (SYS_IMAGE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${SYS_IMAGE_NAME}")
set (SYS_IMAGE_ADDR "0xF000")

set (VID_IMAGE_NAME "revbios.vid")
set (VID_IMAGE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${VID_IMAGE_NAME}")
set (VID_IMAGE_ADDR "0xC000")

add_executable(revbios_sys 
  source/sys/entry.asm 
  source/sys/main.c)
target_link_libraries(revbios_sys PRIVATE ${LINK_LIBRARIES})
target_link_options(revbios_sys BEFORE PRIVATE @${CMAKE_SOURCE_DIR}/source/sys/.link
  option map=${CMAKE_BINARY_DIR}/${SYS_IMAGE_NAME}.map)

add_custom_command(
  OUTPUT ${SYS_IMAGE_NAME}.bin
  COMMAND ${EXE2BIN_CMD} ARGS -l=${SYS_IMAGE_ADDR} -h -r -x $<TARGET_FILE_NAME:revbios_sys> ${SYS_IMAGE_NAME}.bin
  DEPENDS revbios_sys
  COMMENT "Running EXE2BIN on revbios_sys ..."
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(${SYS_IMAGE_NAME}_exe2bin ALL DEPENDS ${SYS_IMAGE_PATH}.bin)

install(FILES ${SYS_IMAGE_PATH}.bin DESTINATION ${CMAKE_INSTALL_PREFIX}/ROMs)