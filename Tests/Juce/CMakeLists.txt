include(../ComponentRules.cmake)

add_sources(${COMPONENT_NAME}_tests
	AudioBufferTests.cpp
	AudioProcessorGraphTests.cpp
	ChangeBroadcasterTests.cpp
	ChangeListenerTests.cpp
	TimerTests.cpp
	MidiBufferTests.cpp
)
target_include_directories(
		${COMPONENT_NAME}_tests
		PRIVATE
		"${SOURCE_DIRECTORY}"
)