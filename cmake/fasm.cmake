function(add_fasm_target TARGET_NAME SRC_FILE)
  find_program(FASM_EXECUTABLE NAMES fasm)
  
  if(NOT FASM_EXECUTABLE)
    message(FATAL_ERROR "FASM executable not found.")
  endif()

  set(TARGET_BIN "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.bin")
  set(SRC_FULL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE}")

  # Get include file arguments into a list
  set(INCLUDE_FILES ${ARGN})
  
  add_custom_command(
    OUTPUT ${TARGET_BIN}
    COMMAND ${FASM_EXECUTABLE} ${SRC_FULL_PATH} ${TARGET_BIN}
    DEPENDS ${SRC_FILE} ${INCLUDE_FILES}
    COMMENT "Compiling ${SRC_FILE} to ${TARGET_BIN} using FASM"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

  add_custom_target(${TARGET_NAME} ALL DEPENDS ${TARGET_BIN})
  install (FILES  ${TARGET_BIN} DESTINATION ${CMAKE_INSTALL_PREFIX}/ROMs)
endfunction()