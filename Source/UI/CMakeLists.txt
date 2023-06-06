#Open Ephys GUI directory-specific file

#add files in this folder
add_sources(open-ephys 
	ControlPanel.cpp
	ControlPanel.h
	CustomArrowButton.cpp
	CustomArrowButton.h
	DataViewport.cpp
	DataViewport.h
	DefaultConfig.cpp
	DefaultConfig.h
	EditorViewport.cpp
	EditorViewport.h
	EditorViewportButtons.cpp
	EditorViewportButtons.h
	FilenameConfigWindow.cpp
	FilenameConfigWindow.h
	GraphViewer.cpp
	GraphViewer.h
	InfoLabel.cpp
	InfoLabel.h
	PluginInstaller.cpp 
	PluginInstaller.h
	ProcessorList.cpp
	ProcessorList.h
	UIComponent.cpp
	UIComponent.h
)

#add nested directories
add_subdirectory(LookAndFeel)
add_subdirectory(Utils)
