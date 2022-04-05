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

/**

  Sends output to the computer's audio device.

  Every Audio Monitor copies it data into two channels (L and R)
    which are sent to the Audio Node. The Audio Node limits
    the maximum values between -1000 and 1000 (to prevent
    saturation), applies gain and expansion, and streams
    the data to the audio device selected in the audio settings
    interface.

  @see GenericProcessor, AudioEditor

*/
class AudioNode : public GenericProcessor
{
public:

    /** Constructor */
    AudioNode();

    /** Destructor */
    ~AudioNode() { }

    /** Handle incoming data and decide which channels to monitor
    */
    void process(AudioBuffer<float>& buffer) override;

    /** Used to change audio monitoring parameters (such as channels to monitor and volume) while acquisition is active.
    */
    void setParameter(int parameterIndex, float newValue) override;

    /** Creates the AudioEditor (located in the ControlPanel). */
    AudioProcessorEditor* createEditor() override;

    /** Resets the connections prior to a new round of data acquisition. */
    void resetConnections() override;

    /** Establishes a connection between a channel of a GenericProcessor and the AudioNode. */
    void addInputChannel(GenericProcessor* source, int chan);

	/** Updates the audio buffer size*/
	void updatePlaybackBuffer();

    /** Called when the audio output buffer size is changed*/
    void updateBufferSize();

    // expand # of inputs for each connected processor
    void registerProcessor(const GenericProcessor* sourceNode);

    /** A pointer to the AudioNode's editor. */
    std::unique_ptr<AudioEditor> audioEditor;

private:

    Array<int> leftChan;
    Array<int> rightChan;
    float volume;
    float noiseGateLevel; // in microvolts

    Expander expander;

    int connectedProcessors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioNode);

};





#endif  // __AUDIONODE_H_AF61F3C5__
