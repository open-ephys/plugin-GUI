#ifndef STRING_TS_H
#define STRING_TS_H

#include <list>
#include <queue>

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../../Processors/GenericProcessor/GenericProcessor.h"

class StringTS
{
public:
	StringTS();
	std::vector<String> splitString(char sep);
	StringTS(MidiMessage& event);
	String getString();
	StringTS(String S);
	StringTS(String S, int64 ts_software);
	StringTS(const StringTS& s);
	StringTS(unsigned char* buf, int _len, int64 ts_software);
	StringTS& operator=(const StringTS& rhs);


	~StringTS();

	juce::uint8* str;
	int len;
	juce::int64 timestamp;
};

#endif