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

    void process (AudioSampleBuffer& buffer) override;
    void setParameter (int parameterIndex, float newValue) override;

    AudioProcessorEditor* createEditor() override;

    bool hasEditor()                const  override { return true; }
    bool isGeneratesTimestamps()    const  override { return true; }
    bool isReady()                  override;

    int getDefaultNumDataOutputs(DataChannel::DataChannelTypes type, int)        const override;

    float getDefaultSampleRate()        const override;
    float getBitVolts (const DataChannel* chan)   const override;

    void updateSettings() override;
    void setEnabledState (bool t)  override;
	bool enable() override;
	bool disable() override;

    String getFile() const;
    bool setFile (String fullpath);

    bool isFileSupported          (const String& filename) const;
    bool isFileExtensionSupported (const String& ext) const;
    void createEventChannels();
	StringArray getSupportedExtensions() const;

private:
    Array<const EventChannel*> moduleEventChannels;
    unsigned int count = 0;
    
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
    int64 bufferCacheWindow; // the current buffer window to read from readBuffer
    Array<RecordedChannelInfo> channelInfo;

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
