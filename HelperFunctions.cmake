function(add_sources target)
	cmake_policy(PUSH)
	if (POLICY CMP0076) #newer version
		cmake_policy(SET CMP0076 NEW)
	endif()
	
	foreach(src ${ARGN})
		target_sources(${target} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/${src})
	endforeach()
	
	cmake_policy(POP)
endfunction()