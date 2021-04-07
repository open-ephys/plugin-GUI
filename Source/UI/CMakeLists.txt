#Open Ephys GUI direcroty-specific file

#add files in this folder
add_sources(open-ephys 
	ControlPanel.cpp
	ControlPanel.h
	CustomArrowButton.cpp
	CustomArrowButton.h
	DataViewport.cpp
	DataViewport.h
	EditorViewport.cpp
	EditorViewport.h
	EditorViewportActions.cpp
	EditorViewportActions.h
	EditorViewportButtons.cpp
	EditorViewportButtons.h
	GraphViewer.cpp
	GraphViewer.h
	InfoLabel.cpp
	InfoLabel.h
	PluginInstaller.cpp 
	PluginInstaller.h
	ProcessorList.cpp
	ProcessorList.h
	SignalChainManager.cpp
	SignalChainManager.h
	TimestampSourceSelection.cpp
	TimestampSourceSelection.h
	UIComponent.cpp
	UIComponent.h
)

#add nested directories
add_subdirectory(LookAndFeel)
add_subdirectory(Utils)
