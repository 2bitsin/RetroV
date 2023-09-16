macro(preprocess_file target_name input_file output_file)
	# Ensure paths are absolute
	get_filename_component(abs_input_file ${input_file} ABSOLUTE)
	get_filename_component(abs_output_file ${output_file} ABSOLUTE)

	if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		add_custom_command(
			OUTPUT ${abs_output_file}
			COMMAND ${CMAKE_C_COMPILER} -E ${abs_input_file} -o ${abs_output_file}
			DEPENDS ${abs_input_file}
			COMMENT "Preprocessing ${abs_input_file} to ${abs_output_file}"
			VERBATIM
		)
	elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
		add_custom_command(
			OUTPUT ${abs_output_file}
			COMMAND ${CMAKE_C_COMPILER} /EP ${abs_input_file} > ${abs_output_file}
			DEPENDS ${abs_input_file}
			COMMENT "Preprocessing ${abs_input_file} to ${abs_output_file}"
			VERBATIM						
		)
	else()
		message(FATAL_ERROR "Unsupported compiler for preprocessing macro")
	endif()

	add_custom_target(
		${target_name} ALL
		DEPENDS ${abs_output_file}
	)
endmacro()