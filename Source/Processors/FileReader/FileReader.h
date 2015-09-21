/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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


#ifndef __FILEREADER_H_B327D3D2__
#define __FILEREADER_H_B327D3D2__


#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../GenericProcessor/GenericProcessor.h"
#include "FileSource.h"

#define BUFFER_SIZE 1024

/**

  Reads data from a file.

  @see GenericProcessor

*/

class FileReader : public GenericProcessor

{
public:

    FileReader();
    ~FileReader();

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void setParameter(int parameterIndex, float newValue);

    AudioProcessorEditor* createEditor();

    bool hasEditor() const
    {
        return true;
    }

    void updateSettings();

    bool isReady();

    bool isSource()
    {
        return true;
    }

	bool generatesTimestamps()
	{
		return true;
	}

    void enabledState(bool t);

    float getDefaultSampleRate();
    int getNumHeadstageOutputs();
    int getNumEventChannels();
    float getBitVolts(Channel* chan);

    bool setFile(String fullpath);
    String getFile();

private:

    int64 timestamp;

    float currentSampleRate;
    int currentNumChannels;
    int64 currentNumSamples;
    int64 startSample;
    int64 stopSample;
    Array<RecordedChannelInfo> channelInfo;

    int64 currentSample;

    int counter; // for testing purposes only

    ScopedPointer<FileSource> input;

    HeapBlock<int16> readBuffer;

	HashMap<String, int> supportedExtensions;

    void setActiveRecording(int index);
    unsigned int samplesToMilliseconds(int64 samples);
    int64 millisecondsToSamples(unsigned int ms);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileReader);

};


#endif  // __FILEREADER_H_B327D3D2__
