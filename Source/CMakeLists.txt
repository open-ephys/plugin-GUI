#Open Ephys GUI direcroty-specific file

#add files in this folder
add_sources(open-ephys 
	AccessClass.h
	AccessClass.cpp
	CoreServices.h
	CoreServices.cpp
	MainWindow.h
	MainWindow.cpp
	Main.cpp
)

#add nested directories
add_subdirectory(Audio)
add_subdirectory(Processors)
add_subdirectory(UI)
add_subdirectory(Utils)

