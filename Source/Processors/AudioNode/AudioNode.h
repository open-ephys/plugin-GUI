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

#ifndef __AUDIONODE_H_AF61F3C5__
#define __AUDIONODE_H_AF61F3C5__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>

#include "../GenericProcessor/GenericProcessor.h"
#include "AudioEditor.h"
#include "../Dsp/Dsp.h"


class AudioEditor;

/**

  The default processor for sending output to the audio monitor.

  The ProcessorGraph has two default nodes: the AudioNode and the RecordNode.
  Every channel of every processor (that's not a sink or a utility) is automatically
  connected to both of these nodes. The AudioNode is used to filter out channels to be
  sent to the audio output device, which can be selected by the user through the AudioEditor
  (located in the ControlPanel).

  Since the AudioNode exists no matter what, it doesn't appear in the ProcessorList.
  Instead, it's created by the ProcessorGraph at startup.

  Each processor has an "Audio" tab within its channel-selector drawer that determines
  which channels will be monitored. At the moment's there's no centralized way to
  control the channels going to the audio monitor; it all happens in a distributed
  way through the individual processors.

  @see GenericProcessor, AudioEditor

*/

class Expander
{
public:
  Expander();
  void setThreshold(float);
  void setRatio(float);
  void setAttack(float);
  void setRelease(float);
  void reset();

  void process(float* sampleData, int numSamples);

private:
    float   threshold;
    float   attack, release, envelope_decay;
    float   output;
    float   transfer_A, transfer_B;
    float   env, gain;

};

class AudioNode : public GenericProcessor
{
public:

    AudioNode();
    ~AudioNode();


    /** Handle incoming data and decide which channels to monitor
    */
    void process(AudioSampleBuffer& buffer) override;

    /** Used to change audio monitoring parameters (such as channels to monitor and volume) while acquisition is active.
    */
    void setParameter(int parameterIndex, float newValue) override;

    /** Creates the AudioEditor (located in the ControlPanel). */
    AudioProcessorEditor* createEditor() override;

    /** Sets the current channel (in advance of a parameter change). */
    void setChannel(const DataChannel* ch);

    /** Used to turn audio monitoring on and off for individual channels. */
    void setChannelStatus(const DataChannel* ch, bool status);

    /** Resets the connections prior to a new round of data acquisition. */
    void resetConnections() override;

    /** Resets the connections prior to a new round of data acquisition. */
    void enableCurrentChannel(bool) override;

    /** Establishes a connection between a channel of a GenericProcessor and the AudioNode. */
    void addInputChannel(GenericProcessor* source, int chan);

	/** Updates the audio buffer size*/
	void updatePlaybackBuffer();

    /** A pointer to the AudioNode's editor. */
    ScopedPointer<AudioEditor> audioEditor;

    void updateBufferSize();

    void prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock) override;

    void updateFilter(int i);

	bool enable() override;

	//Called by ProcessorGraph
	void updateRecordChannelIndexes();

private:
	void recreateBuffers();

    Array<int> leftChan;
    Array<int> rightChan;
    float volume;
    float noiseGateLevel; // in microvolts

    OwnedArray<AudioSampleBuffer> bufferA;
    OwnedArray<AudioSampleBuffer> bufferB;

    Array<int> numSamplesExpected;

    Array<int> samplesInBackupBuffer;
    Array<int> samplesInOverflowBuffer;
    Array<double> sourceBufferSampleRate;
    double destBufferSampleRate;
	int estimatedSamples;

    Array<bool> bufferSwap;

    Expander expander;

    // sample rate, timebase, and ratio info:
    Array<double> ratio;

    // major objects:
    OwnedArray<Dsp::Filter> filters;

    // Temporary buffer for data
    ScopedPointer<AudioSampleBuffer> tempBuffer;

	//private map for datachannels with info relative to multiple processors
	std::unordered_map<uint16, std::map<uint16, int>> audioDataChannelMap;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioNode);

};





#endif  // __AUDIONODE_H_AF61F3C5__
