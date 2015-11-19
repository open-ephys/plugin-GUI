/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef PLACEHOLDERPROCESSOR_H_INCLUDED
#define PLACEHOLDERPROCESSOR_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"

class PlaceholderProcessor : public GenericProcessor
{
public:
	PlaceholderProcessor(String pName, String lName, int lVer, bool pSource, bool pSink);
	~PlaceholderProcessor();
	bool hasEditor() const override;
	AudioProcessorEditor* createEditor() override;

	void process(AudioSampleBuffer& continuousBuffer,
		MidiBuffer& eventBuffer) override;
	bool isSource() override;
	bool isSink() override;
	bool isReady() override;
private:
	const String processorName;
	const String libName;
	const int libVersion;
	const bool processorSource;
	const bool processorSink;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaceholderProcessor);
};



#endif  // PLACEHOLDERPROCESSOR_H_INCLUDED
