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

#include "AudioResamplingNode.h"
#include <stdio.h>
#include "../../Utils/Utils.h"

AudioResamplingNode::AudioResamplingNode()
    : GenericProcessor("Resampling Node"),
      sourceBufferSampleRate(40000.0), destBufferSampleRate(44100.0),
      ratio(1.0), lastRatio(1.0), destBuffer(0), tempBuffer(0),
      destBufferIsTempBuffer(true), isTransmitting(false), destBufferPos(0)
{

    settings.numInputs = 2;
    settings.numOutputs = 2;

    setPlayConfigDetails(getNumInputs(), // number of inputs
                         getNumOutputs(), // number of outputs
                         44100.0, // sampleRate
                         128);    // blockSize

    filter = new Dsp::SmoothedFilterDesign
    <Dsp::RBJ::Design::LowPass, 1> (1024);

    if (destBufferIsTempBuffer)
        destBufferWidth = 1024;
    else
        destBufferWidth = 1000;

    destBufferTimebaseSecs = 1.0;

    destBuffer = new AudioSampleBuffer(16, destBufferWidth);
    tempBuffer = new AudioSampleBuffer(16, destBufferWidth);

    continuousDataBuffer = new int16[10000];

}

AudioResamplingNode::~AudioResamplingNode()
{
    delete[] continuousDataBuffer;
    deleteAndZero(tempBuffer);
    deleteAndZero(destBuffer);
    delete filter;
}



void AudioResamplingNode::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);

    switch (parameterIndex)
    {

        case 0:
            destBufferTimebaseSecs = newValue;
            break;
        case 1:
            destBufferWidth = roundToInt(newValue);

    }

    // reset to zero and clear if timebase or width has changed.
    destBufferPos = 0;
    destBuffer->clear();

}


void AudioResamplingNode::prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock)
{

LOGDD("AudioResamplingNode preparing to play.");

    if (destBufferIsTempBuffer)
    {
        destBufferSampleRate = sampleRate_;
        tempBuffer->setSize(getNumInputs(), 4096);
    }
    else
    {
        destBufferSampleRate = float(destBufferWidth) / destBufferTimebaseSecs;
        destBuffer->setSize(getNumInputs(), destBufferWidth);
    }

    destBuffer->clear();
    tempBuffer->clear();

    destBufferPos = 0;

LOGDD("Temp buffer size: ", tempBuffer->getNumChannels(), " x ");
    //           << tempBuffer->getNumSamples() << std::endl;

    updateFilter();

}

void AudioResamplingNode::updateFilter()
{

    double cutoffFreq = (ratio > 1.0) ? 2 * destBufferSampleRate  // downsample
                        : destBufferSampleRate / 2; // upsample

    double sampleFreq = (ratio > 1.0) ? sourceBufferSampleRate // downsample
                        : destBufferSampleRate;  // upsample

    Dsp::Params params;
    params[0] = sampleFreq; // sample rate
    params[1] = cutoffFreq; // cutoff frequency
    params[2] = 1.25; //Q //

    filter->setParams(params);

}

void AudioResamplingNode::releaseResources()
{

}

void AudioResamplingNode::process(AudioSampleBuffer& buffer,
                                  MidiBuffer& midiMessages)
{

    int nSamps = buffer.getNumSamples(); // WRONG!!!!
    int valuesNeeded;

    if (destBufferIsTempBuffer)
    {
        ratio = float(nSamps) / float(buffer.getNumSamples());
        valuesNeeded = buffer.getNumSamples();
    }
    else
    {
        ratio = sourceBufferSampleRate / destBufferSampleRate;
        valuesNeeded = (int) buffer.getNumSamples() / ratio;
        //std::cout << std::endl;
LOGDD("Ratio: ", ratio);
LOGDD("Values needed: ", valuesNeeded);
    }



    if (lastRatio != ratio)
    {
        updateFilter();
        lastRatio = ratio;
    }


    if (ratio > 1.0001)
    {
        // pre-apply filter before downsampling
        filter->process(nSamps, buffer.getArrayOfWritePointers());
    }


    // initialize variables
    tempBuffer->clear();
    int sourceBufferPos = 0;
    int sourceBufferSize = buffer.getNumSamples();
    float subSampleOffset = 0.0;
    int nextPos = (sourceBufferPos + 1) % sourceBufferSize;

    int tempBufferPos;
    //int totalSamples = 0;

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

LOGDD(sourceBufferPos, " ", tempBufferPos);


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

    if (destBufferIsTempBuffer)
    {

        // copy the temp buffer into the original buffer
        buffer = AudioSampleBuffer(tempBuffer->getArrayOfWritePointers(), 2, tempBufferPos);//buffer.getNumSamples());

    }
    else
    {

LOGDD("Copying into dest buffer...");

        // copy the temp buffer into the destination buffer

        int pos = 0;

        while (*tempBuffer->getWritePointer(0,pos) != 0)
            pos++;

        int spaceAvailable = destBufferWidth - destBufferPos;
        int blockSize1 = (spaceAvailable > pos) ? pos : spaceAvailable;
        int blockSize2 = (spaceAvailable > pos) ? 0 : (pos - spaceAvailable);

        for (int channel = 0; channel < destBuffer->getNumChannels(); channel++)
        {

            // copy first block
            destBuffer->copyFrom(channel, 		//destChannel
                                 destBufferPos, //destStartSample
                                 *tempBuffer, 	//source
                                 channel, 		//sourceChannel
                                 0, 			//sourceStartSample
                                 blockSize1  //numSamples
                                );

            // copy second block
            destBuffer->copyFrom(channel, 		//destChannel
                                 0, 			//destStartSample
                                 *tempBuffer, 	//source
                                 channel, 		//sourceChannel
                                 blockSize1, 	//sourceStartSample
                                 blockSize2  //numSamples
                                );

        }

        //destBufferPos = (spaceAvailable > tempBufferPos) ? destBufferPos

        destBufferPos += pos;
        destBufferPos %= destBufferWidth;

LOGDD("Temp buffer position: ", tempBufferPos);
LOGDD("Resampling node value:", *destBuffer->getReadPointer(0,0));

    }

}


void AudioResamplingNode::writeContinuousBuffer(float* data, int nSamples, int channel)
{

    // find file and write samples to disk
    timestamp = timer.getHighResolutionTicks();

    AudioDataConverters::convertFloatToInt16BE(data, continuousDataBuffer, nSamples);

    //int16 samps = nSamples;

    fwrite(&timestamp,							// ptr
           8,   							// size of each element
           1, 		  						// count
           file);   // ptr to FILE object

    fwrite(&nSamples,								// ptr
           sizeof(nSamples),   				// size of each element
           1, 		  						// count
           file);   // ptr to FILE object

    fwrite(continuousDataBuffer,			// ptr
           2,			     					// size of each element
           nSamples, 		  					// count
           file);   // ptr to FILE object
    // FIXME: check that return value of each fwrite() equals "count";
    // otherwise, there was an error.
}