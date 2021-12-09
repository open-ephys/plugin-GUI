/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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


#define BUFFER_WINDOW_CACHE_SIZE 10


/**
  Reads data from a file.

  @see GenericProcessor
*/
class FileReader : public GenericProcessor,
    private Thread
{
public:
    FileReader();
    ~FileReader();

    void process (AudioBuffer<float>& buffer) override;

    void handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int sampleNum) override;

    String handleConfigMessage(String msg) override;

    void setParameter (int parameterIndex, float newValue) override;

    AudioProcessorEditor* createEditor() override;

    bool generatesTimestamps()    const  override { return true; }

    float getDefaultSampleRate()        const override;

    void updateSettings() override;

	bool startAcquisition() override;
	bool stopAcquisition() override;

    void initialize(bool signalChainIsLoading) override;

    String getFile() const;
    bool setFile (String fullpath);

    bool isFileSupported          (const String& filename) const;
    bool isFileExtensionSupported (const String& ext) const;

	StringArray getSupportedExtensions() const;

    // FileScrubber methods
    int64 getCurrentNumTotalSamples();
    int64 getCurrentNumScrubbedSamples();
    float getCurrentSampleRate() const;
    int64 getCurrentSample();

    void setPlaybackStart(int64 timestamp);
    int getPlaybackStart();

    void setPlaybackStop(int64 timestamp);
    int getPlaybackStop();

    Array<EventInfo> getActiveEventInfo();

    /** Toggles playback on/off */
    void togglePlayback();
    bool playbackIsActive();

    bool loopPlayback;

    unsigned int samplesToMilliseconds (int64 samples)  const;
    int64 millisecondsToSamples (unsigned int ms)       const;

private:
    Array<const EventChannel*> moduleEventChannels;
    ScopedPointer<EventChannel> eventChannel;
    unsigned int count = 0;
    void addEventsInRange(int64 start, int64 stop);

    bool gotNewFile;
    
    void setActiveRecording (int index);

    int64 timestamp;

    float currentSampleRate;
    int currentNumChannels;
    int64 currentSample;
    int64 currentNumTotalSamples;
    int64 currentNumScrubbedSamples;
    int64 startSample;
    int64 stopSample;
    int64 bufferCacheWindow; // the current buffer window to read from readBuffer
    Array<RecordedChannelInfo> channelInfo;
    int64 loopCount;

    bool playbackActive;

    // for testing purposes only
    int counter;

    ScopedPointer<FileSource> input;

    HeapBlock<int16> * readBuffer;      // Ptr to the current "front" buffer
    HeapBlock<int16> bufferA;
    HeapBlock<int16> bufferB;

    HashMap<String, int> supportedExtensions;
    
    Atomic<int> m_shouldFillBackBuffer;
    Atomic<int> m_samplesPerBuffer;

	unsigned int m_bufferSize;
	float m_sysSampleRate;
    
    /** Swaps the backbuffer to the front and flags the background reader
        thread to update the new backbuffer */
    void switchBuffer();
    
    HeapBlock<int16>* getFrontBuffer();
    HeapBlock<int16>* getBackBuffer();
    
    /** Executes the background thread task */
    void run() override;
    
    /** Reads a chunk of the file that fills an entire buffer cache.
     
        This method will read into the buffer that passed in by the param 
     */
    void readAndFillBufferCache(HeapBlock<int16> &cacheBuffer);

	//Methods for built-in file sources
	int getNumBuiltInFileSources() const;

	String getBuiltInFileSourceExtensions(int index) const;

	FileSource* createBuiltInFileSource(int index) const;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileReader);
};


#endif  // __FILEREADER_H_B327D3D2__
