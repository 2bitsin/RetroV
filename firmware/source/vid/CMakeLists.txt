add_executable(vid
  prologue.S 
  main.c)

target_link_libraries(vid PRIVATE com)
set_target_properties(vid PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/.link -Wl,-Map=${CMAKE_BINARY_DIR}/vid.map")
install (TARGETS vid DESTINATION ${CMAKE_INSTALL_PREFIX})

