#Open Ephys GUI direcroty-specific file

#add files in this folder
add_sources(open-ephys 
	architecture.hpp
	compiler.hpp
	os.hpp
	stringify.hpp
	version_number.hpp
)

#add nested directories
add_subdirectory(architecture)
add_subdirectory(compiler)
add_subdirectory(os)


