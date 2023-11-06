cmake_minimum_required(VERSION 3.22)

project(ReVBios C ASM)

set (CMAKE_EXE_LINKER_FLAGS "")


###############
#
#		common
#
###############


set (COM_SOURCE_PATH ${CMAKE_SOURCE_DIR}/source/com)
set (COM_SOURCE_FILES 
  ${COM_SOURCE_PATH}/ulib.c
  ${COM_SOURCE_PATH}/data.c
  ${COM_SOURCE_PATH}/segs.asm)

###############
#
#		revbios_sys
#
###############

set (SYS_IMAGE_NAME revbios_sys)
set (SYS_IMAGE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${SYS_IMAGE_NAME})
set (SYS_SOURCE_PATH ${CMAKE_SOURCE_DIR}/source/sys)
set (SYS_IMAGE_ADDR 0xF000)

add_executable(revbios_sys 
  ${SYS_SOURCE_PATH}/entry.asm 
  ${SYS_SOURCE_PATH}/main.c
  ${SYS_SOURCE_PATH}/xrom.c
  ${COM_SOURCE_FILES})
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

###############
#
#		revbios_vid
#
###############

set (VID_IMAGE_NAME revbios_vid)
set (VID_IMAGE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${VID_IMAGE_NAME})
set (VID_SOURCE_PATH ${CMAKE_SOURCE_DIR}/source/vid)
set (VID_IMAGE_ADDR 0xC000)


add_executable(revbios_vid
  ${VID_SOURCE_PATH}/entry.asm 
  ${VID_SOURCE_PATH}/fonts.asm 
  ${VID_SOURCE_PATH}/main.c
  ${VID_SOURCE_PATH}/int10h.c
  ${VID_SOURCE_PATH}/vgaio.c
  ${COM_SOURCE_FILES})
target_link_libraries(revbios_vid PRIVATE ${WATCOM_LIBRARIES})
target_include_directories(revbios_vid PRIVATE ${CMAKE_SOURCE_DIR}/source ${CMAKE_SOURCE_DIR}/include)
target_link_options(revbios_vid BEFORE PRIVATE @${VID_SOURCE_PATH}/.link
  option map=${CMAKE_BINARY_DIR}/${VID_IMAGE_NAME}.map)


add_custom_command(
  OUTPUT ${VID_IMAGE_NAME}.bin
  COMMAND ${EXE2BIN_CMD} ARGS -l=${VID_IMAGE_ADDR} -h -r -x $<TARGET_FILE_NAME:revbios_vid> ${VID_IMAGE_NAME}.bin
  DEPENDS revbios_vid
  COMMENT "Running EXE2BIN on revbios_vid ..."
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(${VID_IMAGE_NAME}_exe2bin ALL DEPENDS ${VID_IMAGE_PATH}.bin)

install(FILES ${VID_IMAGE_PATH}.bin DESTINATION ${CMAKE_INSTALL_PREFIX} RENAME vid.bin)
