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

#include "../../Utils/Utils.h"


#define BUFFER_WINDOW_CACHE_SIZE 10


/**
  Reads data from a file.

  @see GenericProcessor
*/
class FileReader : 
    public GenericProcessor,
    private Thread
{
public:

    /** Constructor */
    FileReader();

    /** Destructor */
    ~FileReader();

    /** Add latest samples to the signal chain buffer */
    void process (AudioBuffer<float>& buffer) override;

    /** Makes it possible to set the selected file remotely */
    String handleConfigMessage(String msg) override;

    /** Allows parameters to change during acquisition*/
    void setParameter (int parameterIndex, float newValue) override;

    /** Creates the editor */
    AudioProcessorEditor* createEditor() override;

    /** Flags that this processor does generate timestamps */
    bool generatesTimestamps() const override { return true; }

    /** Get the default data sample rate */
    float getDefaultSampleRate() const override;

    /* Updates the FileReader settings*/
    void updateSettings() override;

    /* Called at start of acquisition */
	bool startAcquisition() override;

    /* Called at end of acquisition */
	bool stopAcquisition() override;

    /* Load default example data */
    void initialize(bool signalChainIsLoading) override;

    /* Set the current file */
    bool setFile (String fullpath);

    /* Get the active file's name */
    String getFile() const;

    /* Returns true if input file format is supported */
    bool isFileSupported (const String& filename) const;

    /* Returns a list of file formats supported by the GUI */
	StringArray getSupportedExtensions() const;

    /** Returns the total number of samples per channel */
    int64 getCurrentNumTotalSamples();

    /** Returns a list of EventInfo for the current stream */
    Array<EventInfo> getActiveEventInfo();

    /** Returns the data sample rate of the current stream */
    float getCurrentSampleRate() const;

    /** Returns the current sample (timestamp) */
    int64 getCurrentSample();

    /** Sets the timestamp at which to start playback */
    void setPlaybackStart(int64 timestamp);

    /** Returns the timestamp at which to start playback */
    int getPlaybackStart();

    /** Sets the timestamp at which to stop playback */
    void setPlaybackStop(int64 timestamp);

    /** Returns the timestamp at which to stop playback */
    int getPlaybackStop();

    /** Toggles playback on/off */
    void togglePlayback();

    /** Returns true if playback is currently active */
    bool playbackIsActive();

    /** Flag whether to loop or stop at the end of playback */
    bool loopPlayback;

    /** Converts samples to milliseconds using current stream's sample rate */
    unsigned int samplesToMilliseconds (int64 samples) const;

    /** Converts milliseconds to samples using current stream's sample rate */
    int64 millisecondsToSamples (unsigned int ms) const;

        /** Swaps the backbuffer to the front and flags the background readerthread to update the new backbuffer */
    void switchBuffer();

private:

    /** Currently only support one event channel per stream */
    ScopedPointer<EventChannel> eventChannel;

    /** Generates any events found within the current continuous buffer interval */
    void addEventsInRange(int64 start, int64 stop);

    /** Flag if a new file has been loaded */
    bool gotNewFile;
    
    /** Sets the current stream to read data from */
    void setActiveRecording (int index);

    int64 totalSamplesAcquired;

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

    ScopedPointer<FileSource> input;

    /* Pointer to current front buffer */
    HeapBlock<int16> * readBuffer;      
    HeapBlock<int16> bufferA;
    HeapBlock<int16> bufferB;

    HashMap<String, int> supportedExtensions;
    
    Atomic<int> m_shouldFillBackBuffer;
    Atomic<int> m_samplesPerBuffer;

	unsigned int m_bufferSize;
	float m_sysSampleRate;
    
    HeapBlock<int16>* getFrontBuffer();
    HeapBlock<int16>* getBackBuffer();
    
    /** Executes the background thread task */
    void run() override;
    
    /** Reads a chunk of the file that fills an entire buffer cache. */
    void readAndFillBufferCache(HeapBlock<int16> &cacheBuffer);

	/** Returns the number of included file sources */
	int getNumBuiltInFileSources() const;

    /** Returns the extension for a given file source */
	String getBuiltInFileSourceExtensions(int index) const;

    /** Returns a new FileSource object for a given file source */
	FileSource* createBuiltInFileSource(int index) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileReader);
};


#endif  // __FILEREADER_H_B327D3D2__
