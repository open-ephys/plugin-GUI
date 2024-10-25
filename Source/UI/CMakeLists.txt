#Open Ephys GUI directory-specific file

#add files in this folder
add_sources(open-ephys 
	ConsoleViewer.cpp
	ConsoleViewer.h
	ControlPanel.cpp
	ControlPanel.h
	CustomArrowButton.cpp
	CustomArrowButton.h
	MessageCenterButton.h
	DataViewport.cpp
	DataViewport.h
	DefaultConfig.cpp
	DefaultConfig.h
	MessageWindow.cpp
	MessageWindow.h
	EditorViewport.cpp
	EditorViewport.h
	FilenameConfigWindow.cpp
	FilenameConfigWindow.h
	GraphViewer.cpp
	GraphViewer.h
	InfoLabel.cpp
	InfoLabel.h
	PluginInstaller.cpp 
	PluginInstaller.h
	PopupComponent.cpp
	PopupComponent.h
	ProcessorList.cpp
	ProcessorList.h
	UIComponent.cpp
	UIComponent.h
)

#add nested directories
add_subdirectory(LookAndFeel)
add_subdirectory(Utils)
