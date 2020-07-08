#Open Ephys GUI direcroty-specific file

#add files in this folder
add_sources(open-ephys 
	taskflow.hpp
)

#add nested directories
add_subdirectory(core)
add_subdirectory(error)
add_subdirectory(predef)
add_subdirectory(utility)

