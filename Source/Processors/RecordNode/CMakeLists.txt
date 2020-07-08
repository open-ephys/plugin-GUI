#Open Ephys GUI direcroty-specific file
set_property(TARGET open-ephys PROPERTY CXX_STANDARD 17)

#add files in this folder
add_sources(open-ephys 
	DataQueue.cpp
	DataQueue.h
	EngineConfigWindow.cpp
	EngineConfigWindow.h
	EventQueue.h
	RecordChannelSelector.h
	RecordChannelSelector.cpp
	RecordEngine.cpp
	RecordEngine.h
	RecordNode.cpp
	RecordNode.h
	RecordNodeEditor.cpp
	RecordNodeEditor.h
	RecordThread.cpp
	RecordThread.h
	SyncChannelSelector.cpp
	SyncChannelSelector.h
	Synchronizer.cpp
	Synchronizer.h
	Utils.h
)

#add nested directories
add_subdirectory(BinaryFormat)
add_subdirectory(OpenEphysFormat)
add_subdirectory(taskflow)