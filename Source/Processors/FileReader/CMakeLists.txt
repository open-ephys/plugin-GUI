#Open Ephys GUI direcroty-specific file

#add files in this folder
add_sources(open-ephys 
	FileReader.cpp
	FileReader.h
	FileReaderEditor.cpp
	FileReaderEditor.h
	FileSource.cpp
	FileSource.h
)

#add nested directories
add_subdirectory(BinaryFileSource)

