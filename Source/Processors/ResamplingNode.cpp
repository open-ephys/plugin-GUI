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

#include "ResamplingNode.h"
#include "Editors/ResamplingNodeEditor.h"

#include <stdio.h>

ResamplingNode::ResamplingNode()
    : GenericProcessor("Resampler"),
      targetSampleRate(5000.0f), ratio(1.0)
{

    filter = new Dsp::SmoothedFilterDesign
    <Dsp::RBJ::Design::LowPass, 1> (1024);

    parameters.add(Parameter("Hz",500.0f, 10000.0f, targetSampleRate, 0, true));

    tempBuffer = new AudioSampleBuffer(16, TEMP_BUFFER_WIDTH);

}

ResamplingNode::~ResamplingNode()
{
    filter = 0;
}

AudioProcessorEditor* ResamplingNode::createEditor()
{
    editor = new ResamplingNodeEditor(this, true);

    //std::cout << "Creating editor." << std::endl;

    return editor;
}

void ResamplingNode::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);

    if (parameterIndex == 0)
    {
        Parameter& p =  parameters.getReference(parameterIndex);
        p.setValue(newValue, 0);

        targetSampleRate = newValue;

        settings.sampleRate = targetSampleRate;

        ratio = sourceBufferSampleRate / targetSampleRate;

        for (int i = 0; i < channels.size(); i++)
        {
            channels[i]->sampleRate = targetSampleRate;
        }

        updateFilter();

        //std::cout << "Got parameter update." << std::endl;
    }

    //std::cout << float(p[0]) << std::endl;

}

bool ResamplingNode::enable()
{


    tempBuffer->clear();

    updateFilter();

    return true;

}

void ResamplingNode::updateSettings()
{

    sourceBufferSampleRate = settings.sampleRate;
    settings.sampleRate = targetSampleRate;

    if (getNumInputs() > 0)
        tempBuffer->setSize(getNumInputs(), TEMP_BUFFER_WIDTH);

    ratio = sourceBufferSampleRate / targetSampleRate;

    for (int i = 0; i < channels.size(); i++)
    {
        channels[i]->sampleRate = targetSampleRate;
    }

    updateFilter();

}


void ResamplingNode::updateFilter()
{

    double cutoffFreq = (ratio > 1.0) ? 2 * targetSampleRate  // downsample
                        : targetSampleRate / 2; // upsample

    double sampleFreq = (ratio > 1.0) ? sourceBufferSampleRate // downsample
                        : targetSampleRate;  // upsample

    Dsp::Params params;
    params[0] = sampleFreq; // sample rate
    params[1] = cutoffFreq; // cutoff frequency
    params[2] = 1.25; //Q //

    filter->setParams(params);

}

// void ResamplingNode::releaseResources()
// {
// 	//fclose(file);
// }

void ResamplingNode::process(AudioSampleBuffer& buffer,
                             MidiBuffer& midiMessages,
                             int& nSamples)
{

    //std::cout << "Resampling node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;

    // save data at the beginning of each round of processing
    //writeContinuousBuffer(buffer.getSampleData(0), nSamples, 0);


    int nSamps = float(nSamples);
    int valuesNeeded = int(nSamps / ratio);

    if (ratio > 1.0001)
    {
        // pre-apply filter before downsampling
        filter->process(nSamples, buffer.getArrayOfWritePointers());
    }

    // initialize variables
    tempBuffer->clear();
    int sourceBufferPos = 0;
    int sourceBufferSize = buffer.getNumSamples();
    float subSampleOffset = 0.0;
    int nextPos = (sourceBufferPos + 1) % sourceBufferSize;

    int tempBufferPos;

    // code modified from "juce_ResamplingAudioSource.cpp":

    for (tempBufferPos = 0; tempBufferPos < valuesNeeded; tempBufferPos++)
    {
        float gain = 1.0;
        float alpha = (float) subSampleOffset;
        float invAlpha = 1.0f - alpha;

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {

            tempBuffer->addFrom(channel, 		// destChannel
                                tempBufferPos,  // destSampleOffset
                                buffer,			// source
                                channel,		// sourceChannel
                                sourceBufferPos,// sourceSampleOffset
                                1,				// number of samples
                                invAlpha*gain);      // gain to apply to source

            tempBuffer->addFrom(channel, 		// destChannel
                                tempBufferPos,   // destSampleOffset
                                buffer,			// source
                                channel,			// sourceChannel
                                nextPos,		 	// sourceSampleOffset
                                1,				// number of samples
                                alpha*gain);     	 // gain to apply to source

        }

        subSampleOffset += ratio;

        while (subSampleOffset >= 1.0)
        {
            if (++sourceBufferPos >= sourceBufferSize)
                sourceBufferPos = 0;

            nextPos = (sourceBufferPos + 1) % sourceBufferSize;
            subSampleOffset -= 1.0;
        }
    }

    // std::cout << sourceBufferPos << " " << tempBufferPos << std::endl;


    if (ratio < 0.9999)
    {

        filter->process(tempBufferPos, tempBuffer->getArrayOfWritePointers());
        // apply the filter after upsampling
        ///////filter->process (totalSamples, buffer.getArrayOfChannels());
    }
    else if (ratio <= 1.0001)
    {

        // no resampling is being applied, no need to filter, BUT...
        // keep the filter stoked with samples to avoid discontinuities

    }

    // copy the tempBuffer back into the original buffer
    buffer = AudioSampleBuffer(tempBuffer->getArrayOfWritePointers(), 2, tempBufferPos);//buffer.getNumSamples());

    nSamples = valuesNeeded;

}
