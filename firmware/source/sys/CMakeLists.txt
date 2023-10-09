add_executable(sys 	
  startup.S
  prologue.S 
	int10h.c
  main.c)

target_link_libraries(sys PRIVATE com)
set_target_properties(sys PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/.link -Wl,-Map=${CMAKE_BINARY_DIR}/sys.map")
install (TARGETS sys DESTINATION ${CMAKE_INSTALL_PREFIX})
