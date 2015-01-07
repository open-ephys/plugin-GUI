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

#ifndef __RESAMPLINGNODE_H_79663B0__
#define __RESAMPLINGNODE_H_79663B0__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Dsp/Dsp.h"
#include "../GenericProcessor/GenericProcessor.h"

#define TEMP_BUFFER_WIDTH 5000

/**

  Changes the sample rate of continuous data.

  Code is based on Juce's ResamplingAudioSource class.

  @see GenericProcessor

*/

class ResamplingNode : public GenericProcessor

{
public:

    // real member functions:
    ResamplingNode();
    ~ResamplingNode();

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void setParameter(int parameterIndex, float newValue);

    void updateSettings();

    void updateFilter();

    bool enable();

    AudioProcessorEditor* createEditor();
    bool hasEditor() const
    {
        return true;
    }

private:

    // sample rate, timebase, and ratio info:
    double targetSampleRate;
    double sourceBufferSampleRate; //, destBufferSampleRate;
    double ratio; //, lastRatio;
    //double destBufferTimebaseSecs;
    //int destBufferWidth;

    // major objects:
    Dsp::Filter* filter;
    //ScopedPointer<AudioSampleBuffer> destBuffer;
    ScopedPointer<AudioSampleBuffer> tempBuffer;

    // is the destBuffer a temp buffer or not?
    //bool destBufferIsTempBuffer;
    //bool isTransmitting;

    // indexing objects that persist between rounds:
    //int destBufferPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResamplingNode);

};




#endif  // __RESAMPLINGNODE_H_79663B0__
