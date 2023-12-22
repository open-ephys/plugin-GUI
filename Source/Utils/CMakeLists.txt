#Open Ephys GUI direcroty-speciric file

add_sources(open-ephys
	OpenEphysHttpServer.h
	ListSliceParser.h
	ListSliceParser.cpp
	BroadcastParser.h
	BroadcastParser.cpp
	BroadcastPayload.h
	BroadcastPayload.cpp
	Utils.h
)

# Dependent targets in the same CMake tree may have defined this cpp-httplib target already. To prevent conflicts
# of different versions, try to utilize a single one here if it already exists. See OpenEphysHttpServer.h for use of the
# below macro.
if (TARGET httplib::httplib)
	target_link_libraries(open-ephys httplib::httplib)
else()
	target_compile_definitions(open-ephys OPENEPHYS_INCLUDE_HTTPLIB)
endif()

#add nested directories


