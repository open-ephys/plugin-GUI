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

    void process (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void setParameter (int parameterIndex, float newValue);

    AudioProcessorEditor* createEditor();

    // We should make all these override methods as constant
    // so we also need to refactor GenericProcessor and appropriate plugins
    bool hasEditor()                const       override { return true; }
    bool isSource()                 /* const */ override { return true; }
    bool generatesTimestamps()      /* const */ override { return true; }
    bool isReady()                  /* const */ override;

    int getNumHeadstageOutputs()        /* const */ override;
    int getNumEventChannels()           /* const */ override;
    float getDefaultSampleRate()        /* const */ override;
    float getBitVolts (Channel* chan)   /* const */ override;

    void updateSettings()       override;
    void enabledState (bool t)  override;

    String getFile() const;
    bool setFile (String fullpath);

    bool isFileSupported          (const String& filename) const;
    bool isFileExtensionSupported (const String& ext) const;


private:
    void setActiveRecording (int index);

    unsigned int samplesToMilliseconds (int64 samples)  const;
    int64 millisecondsToSamples (unsigned int ms)       const;


    int64 timestamp;

    float currentSampleRate;
    int currentNumChannels;
    int64 currentSample;
    int64 currentNumSamples;
    int64 startSample;
    int64 stopSample;
    Array<RecordedChannelInfo> channelInfo;

    // for testing purposes only
    int counter;

    ScopedPointer<FileSource> input;

    HeapBlock<int16> readBuffer;

    HashMap<String, int> supportedExtensions;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileReader);
};


#endif  // __FILEREADER_H_B327D3D2__
