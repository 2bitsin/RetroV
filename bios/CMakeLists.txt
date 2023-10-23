cmake_minimum_required(VERSION 3.22)

project(ReVBios C ASM)

set (CMAKE_EXE_LINKER_FLAGS "")

set (SYS_IMAGE_NAME revbios_sys)
set (SYS_IMAGE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${SYS_IMAGE_NAME})
set (SYS_SOURCE_PATH ${CMAKE_SOURCE_DIR}/source/sys)
set (SYS_IMAGE_ADDR 0xF000)

set (VID_IMAGE_NAME revbios_vid)
set (VID_IMAGE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${VID_IMAGE_NAME})
set (VID_SOURCE_PATH ${CMAKE_SOURCE_DIR}/source/vid)
set (VID_IMAGE_ADDR 0xC000)

add_executable(revbios_sys 
  ${SYS_SOURCE_PATH}/entry.asm 
  ${SYS_SOURCE_PATH}/main.c)
target_link_libraries(revbios_sys PRIVATE ${WATCOM_LIBRARIES})
target_include_directories(revbios_sys PRIVATE ${CMAKE_SOURCE_DIR}/source ${CMAKE_SOURCE_DIR}/include)
target_link_options(revbios_sys BEFORE PRIVATE @${SYS_SOURCE_PATH}/.link
  option map=${CMAKE_BINARY_DIR}/${SYS_IMAGE_NAME}.map)

add_custom_command(
  OUTPUT ${SYS_IMAGE_NAME}.bin
  COMMAND ${EXE2BIN_CMD} ARGS -l=${SYS_IMAGE_ADDR} -h -r -x $<TARGET_FILE_NAME:revbios_sys> ${SYS_IMAGE_NAME}.bin
  DEPENDS revbios_sys
  COMMENT "Running EXE2BIN on revbios_sys ..."
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(${SYS_IMAGE_NAME}_exe2bin ALL DEPENDS ${SYS_IMAGE_PATH}.bin)

install(FILES ${SYS_IMAGE_PATH}.bin DESTINATION ${CMAKE_INSTALL_PREFIX} RENAME sys.bin)