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

function(copy_sources src_target dest_target)
	cmake_policy(PUSH)
	if (POLICY CMP0076) #newer version
		cmake_policy(SET CMP0076 NEW)
	endif()
	
	unset(SRC_TARGET_FILES)
	get_target_property(SRC_TARGET_FILES ${src_target} SOURCES)
	target_sources(${dest_target} PRIVATE ${SRC_TARGET_FILES})	
	
	cmake_policy(POP)
endfunction()
