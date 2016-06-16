#include "StringTS.h"

StringTS::StringTS()
{

	str = nullptr;
	len = 0;
	timestamp = 0;
}


std::vector<String> StringTS::splitString(char sep)
{
	String S((const char*)str, len);
	std::list<String> ls;
	String  curr;
	for (int k = 0; k < S.length(); k++)
	{
		if (S[k] != sep)
		{
			curr += S[k];
		}
		else
		{
			ls.push_back(curr);
			while (S[k] == sep && k < S.length())
				k++;

			curr = "";
			if (S[k] != sep && k < S.length())
				curr += S[k];
		}
	}
	if (S.length() > 0)
	{
		if (S[S.length() - 1] != sep)
			ls.push_back(curr);
	}
	std::vector<String> Svec(ls.begin(), ls.end());
	return Svec;

}

StringTS::StringTS(MidiMessage& event)
{
	const uint8* dataptr = event.getRawData();
	int bufferSize = event.getRawDataSize();
	len = bufferSize - 20; 
	str = new uint8[len];
	memcpy(str, dataptr+6, len);

	memcpy(&timestamp, dataptr +6+ len, 8); 
	
}


StringTS& StringTS::operator=(const StringTS& rhs)
{
	delete(str);
	len = rhs.len;
	str = new uint8[len];
	memcpy(str, rhs.str, len);
	timestamp = rhs.timestamp;

	return *this;
}

String StringTS::getString()
{
	return String((const char*)str, len);
}

StringTS::StringTS(String S)
{
	Time t;
	str = new uint8[S.length()];
	memcpy(str, S.toRawUTF8(), S.length());
	timestamp = t.getHighResolutionTicks();

	len = S.length();
}

StringTS::StringTS(String S, int64 ts_software)
{
	str = new uint8[S.length()];
	memcpy(str, S.toRawUTF8(), S.length());
	timestamp = ts_software;

	len = S.length();
}

StringTS::StringTS(const StringTS& s)
{
	str = new uint8[s.len];
	memcpy(str, s.str, s.len);
	timestamp = s.timestamp;
	len = s.len;
}


StringTS::StringTS(unsigned char* buf, int _len, int64 ts_software) : len(_len), timestamp(ts_software)
{
	str = new juce::uint8[len];
	for (int k = 0; k<len; k++)
		str[k] = buf[k];
}

StringTS::~StringTS()
{
	delete str;
}
