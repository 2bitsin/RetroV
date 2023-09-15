function(add_fasm_target TARGET_NAME MAIN_SRC)
  
  find_program(FASM_EXECUTABLE fasm)
  
  if(NOT FASM_EXECUTABLE)
    message(FATAL_ERROR "FASM executable not found.")
  endif()

  set(TARGET_BIN "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.bin")
  set(MAIN_SRC_FULL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_SRC}")

  # Extracting dependency files
  set(DEPENDENCIES ${ARGN}) 
  
  list(APPEND Q "$<TARGET_PROPERTY:${TARGET_NAME},DEFS>")
  list(APPEND Q ${MAIN_SRC_FULL_PATH})
  list(APPEND Q ${TARGET_BIN})
  add_custom_command(
    OUTPUT ${TARGET_BIN}
    COMMAND ${FASM_EXECUTABLE} ARGS ${Q}
    DEPENDS ${MAIN_SRC} ${DEPENDENCIES}
    COMMENT "Compiling ${MAIN_SRC} to ${TARGET_BIN} using FASM"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND_EXPAND_LISTS
    VERBATIM
  )
  add_custom_target(${TARGET_NAME} ALL DEPENDS ${TARGET_BIN})  
  set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_FILE_PATH ${TARGET_BIN})
endfunction()

function(add_fasm_option TARGET_NAME)
  set_property(TARGET ${TARGET_NAME} PROPERTY DEFS ${ARGN})
endfunction()