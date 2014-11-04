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

#include <cmath>

#include "AudioNode.h"

AudioNode::AudioNode()
    : GenericProcessor("Audio Node"), audioEditor(0), volume(0.00001f), noiseGateLevel(0.0f)
{

    settings.numInputs = 4096;
    settings.numOutputs = 2;

    // 128 inputs, 2 outputs (left and right channel)
    setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

    nextAvailableChannel = 2; // keep first two channels empty

    tempBuffer = new AudioSampleBuffer(16, 1024);

}


AudioNode::~AudioNode()
{
}

AudioProcessorEditor* AudioNode::createEditor()
{

    audioEditor = new AudioEditor(this);

    return audioEditor;

}

void AudioNode::resetConnections()
{

    nextAvailableChannel = 2; // start connections at channel 2
    wasConnected = false;

    channelPointers.clear();

}

void AudioNode::updateBufferSize()
{
    //AudioEditor* editor = (AudioEditor*) getEditor();
    audioEditor->updateBufferSizeText();

}

void AudioNode::setChannel(Channel* ch)
{

    int channelNum = channelPointers.indexOf(ch);

    std::cout << "Audio node setting channel to " << channelNum << std::endl;

    setCurrentChannel(channelNum);
}

void AudioNode::setChannelStatus(Channel* chan, bool status)
{

    setChannel(chan); // add 2 to account for 2 output channels

    enableCurrentChannel(status);

}

void AudioNode::enableCurrentChannel(bool state)
{

    if (state)
    {
        setParameter(100, 0.0f);
    }
    else
    {
        setParameter(-100, 0.0f);
    }
}


void AudioNode::addInputChannel(GenericProcessor* sourceNode, int chan)
{


    int channelIndex = getNextChannel(false);

    setPlayConfigDetails(channelIndex+1,0,44100.0,128);

    channelPointers.add(sourceNode->channels[chan]);

}

void AudioNode::setParameter(int parameterIndex, float newValue)
{
    // change left channel, right channel, or volume
    if (parameterIndex == 1)
    {
        // volume level
        volume = newValue*0.1f;

    }
    else if (parameterIndex == 2)
    {
        // noiseGateLevel level

        expander.setThreshold(newValue); // in microVolts

    }
    else if (parameterIndex == 100)
    {

        channelPointers[currentChannel]->isMonitored = true;

    }
    else if (parameterIndex == -100)
    {

        channelPointers[currentChannel]->isMonitored = false;
    }

}

void AudioNode::prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock)
{


    // std::cout << "Processor sample rate: " << getSampleRate() << std::endl;
    // std::cout << "Audio card sample rate: " << sampleRate_ << std::endl;
    // std::cout << "Samples per block: " << estimatedSamplesPerBlock << std::endl;

    numSamplesExpected.clear();
    samplesInBackupBuffer.clear();
    samplesInOverflowBuffer.clear();
    ratio.clear();
    filters.clear();
    bufferA.clear();
    bufferB.clear();

    destBufferSampleRate = sampleRate_;

    for (int i = 0; i < channelPointers.size(); i++)
    {
        // processor sample rate divided by sound card sample rate
        numSamplesExpected.add((int)(channelPointers[i]->sampleRate/sampleRate_*float(estimatedSamplesPerBlock)) + 1);
        samplesInBackupBuffer.add(0);
        samplesInOverflowBuffer.add(0);
        sourceBufferSampleRate.add(channelPointers[i]->sampleRate);

        filters.add(new Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::LowPass, 1> (1024));
        
        ratio.add(float(numSamplesExpected[i])/float(estimatedSamplesPerBlock)); 
        updateFilter(i);

        bufferA.add(new AudioSampleBuffer(1,10000));
        bufferB.add(new AudioSampleBuffer(1,10000));
      
    }
    
    tempBuffer->setSize(getNumInputs(), 4096);

    bufferSwap = false;
}

void AudioNode::updateFilter(int i)
{

    double cutoffFreq = (ratio[i] > 1.0) ? 2 * destBufferSampleRate  // downsample
                        : destBufferSampleRate / 2; // upsample

    double sampleFreq = (ratio[i] > 1.0) ? sourceBufferSampleRate[i] // downsample
                        : destBufferSampleRate;  // upsample

    Dsp::Params params;
    params[0] = sampleFreq; // sample rate
    params[1] = cutoffFreq; // cutoff frequency
    params[2] = 1.25; //Q //

    filters[i]->setParams(params);

}

void AudioNode::process(AudioSampleBuffer& buffer,
                        MidiBuffer& events)
{
  float gain;
  int valuesNeeded = buffer.getNumSamples(); // samples needed to fill out the buffer

  // clear the left and right channels
  buffer.clear(0,0,buffer.getNumSamples());
  buffer.clear(1,0,buffer.getNumSamples());

  if (1)
  {

    AudioSampleBuffer* overflowBuffer;
    AudioSampleBuffer* backupBuffer;

    if (channelPointers.size() > 0) // we have some channels
    {

      for (int i = 0; i < buffer.getNumChannels()-2; i++)
      {

        if (channelPointers[i]->isMonitored)
        {

          if (!bufferSwap)
          {
            overflowBuffer = bufferA[i];
            backupBuffer = bufferB[i];

            if (i == buffer.getNumChannels()-3)
              bufferSwap = true;
          }
          else
          {
            overflowBuffer = bufferB[i];
            backupBuffer = bufferA[i];

            if (i == buffer.getNumChannels()-3)
              bufferSwap = false;
          }

          backupBuffer->clear();
          tempBuffer->clear();

          samplesInOverflowBuffer.set(i,samplesInBackupBuffer[i]); // size of buffer after last round
          samplesInBackupBuffer.set(i,0);

          int orphanedSamples = 0;

          // 1. copy overflow buffer

          int samplesToCopy = ((samplesInOverflowBuffer[i] <= numSamplesExpected[i]) ?
                           samplesInOverflowBuffer[i] :
                           numSamplesExpected[i]);

          //  std::cout << " " << std::endl;
          //  std::cout << "Copying " << samplesToCopy << " samples from overflow buffer of " << samplesInOverflowBuffer << " samples." << std::endl;

          if (samplesToCopy > 0)
          {

            tempBuffer->addFrom(i,    // destination channel
                           0,                // destination start sample
                           *overflowBuffer,  // source
                           i,                // source channel
                           0,                // source start sample
                           samplesToCopy,    // number of samples
                           1.0f              // gain to apply
                          );

            int leftoverSamples = samplesInOverflowBuffer[i] - samplesToCopy;

            //     std::cout << "Samples remaining in overflow buffer: " << leftoverSamples << std::endl;

            if (leftoverSamples > 0)
            {

              // move remaining samples to the backup buffer

              backupBuffer->addFrom(i, // destination channel
                                    0,                     // destination start sample
                                    *overflowBuffer,       // source
                                    i,                     // source channel
                                    samplesToCopy,         // source start sample
                                    leftoverSamples,       // number of samples
                                    1.0f                   // gain to apply
                                   );

            }

            samplesInBackupBuffer.set(i,leftoverSamples);

            gain = volume/(float(0x7fff) * channelPointers[i]->bitVolts);
            // Data are floats in units of microvolts, so dividing by bitVolts and 0x7fff (max value for 16b signed)
            // rescales to between -1 and +1. Audio output starts So, maximum gain applied to maximum data would be 10.

            int remainingSamples = numSamplesExpected[i] - samplesToCopy;

            //  std::cout << "Copying " << remainingSamples << " samples from incoming buffer of " << nSamples << " samples." << std::endl;

            int samplesAvailable = numSamples.at(channelPointers[i-2]->sourceNodeId);

            int samplesToCopy2 = ((remainingSamples <= samplesAvailable) ?
                                  remainingSamples :
                                  samplesAvailable);

            if (samplesToCopy2 > 0)
            {

              tempBuffer->addFrom(i,       // destination channel
                             samplesToCopy,           // destination start sample
                             buffer,      // source
                             i+2,           // source channel (add 2 to account for output channels)
                             0,           // source start sample
                             remainingSamples, //  number of samples
                             gain       // gain to apply
                            );

            }

            orphanedSamples = samplesAvailable - samplesToCopy2;

            // std::cout << "Samples remaining in incoming buffer: " << orphanedSamples << std::endl;

            if (orphanedSamples > 0 && (samplesInBackupBuffer[i] + orphanedSamples < backupBuffer->getNumSamples()))
            {

                backupBuffer->addFrom(i,                            // destination channel
                                      samplesInBackupBuffer[i],     // destination start sample
                                      buffer,                       // source
                                      i+2,                          // source channel (add 2 to account for output channels)
                                      remainingSamples,             // source start sample
                                      orphanedSamples,              //  number of samples
                                      gain                          // gain to apply
                                     );

					  }
    				else 
            {
    						samplesInBackupBuffer.set(i,0); // just throw out the buffer in the case of an overrun
    											                 // not ideal, but the output still sounds fine
    				}

            // now that our tempBuffer is ready, we can filter it and copy it into the 
            // original buffer


            if (ratio[i] > 1.0001)
            {
                // pre-apply filter before downsampling
                float* ptr = tempBuffer->getWritePointer(i+2);
                filters[i]->process(numSamplesExpected[i], &ptr);
            }

            // initialize variables
            int sourceBufferPos = 0;
            int sourceBufferSize = numSamplesExpected[i];
            float subSampleOffset = 0.0;
            int nextPos = (sourceBufferPos + 1) % sourceBufferSize;

            int destBufferPos;
            buffer.clear(i+2,0,valuesNeeded);

            // code modified from "juce_ResamplingAudioSource.cpp":

            for (destBufferPos = 0; destBufferPos < valuesNeeded; destBufferPos++)
            {
              float gain = 1.0;
              float alpha = (float) subSampleOffset;
              float invAlpha = 1.0f - alpha;

              buffer.addFrom(i+2,    // destChannel
                             destBufferPos,  // destSampleOffset
                             *tempBuffer,     // source
                              i,    // sourceChannel
                              sourceBufferPos,// sourceSampleOffset
                              1,        // number of samples
                              invAlpha*gain);      // gain to apply to source

              buffer.addFrom(i+2,    // destChannel
                              destBufferPos,   // destSampleOffset
                              buffer,     // source
                              i,      // sourceChannel
                              nextPos,      // sourceSampleOffset
                              1,        // number of samples
                              alpha*gain);       // gain to apply to source

              subSampleOffset += ratio[i];

              while (subSampleOffset >= 1.0)
              {
                  if (++sourceBufferPos >= sourceBufferSize)
                      sourceBufferPos = 0;

                  nextPos = (sourceBufferPos + 1) % sourceBufferSize;
                  subSampleOffset -= 1.0;
              }
            }

            if (ratio[i] < 0.9999)
            {
                float* ptr = buffer.getWritePointer(i+2);
                filters[i]->process(destBufferPos, &ptr);
                // apply the filter after upsampling
            }

            // now copy the channel into the output zone

            buffer.addFrom(0,    // destChannel
                           0,  // destSampleOffset
                           buffer,     // source
                            i+2,    // sourceChannel
                            0,// sourceSampleOffset
                            valuesNeeded,        // number of samples
                            1.0);      // gain to apply to source

          } // if samplesToCopy > 0
        } // if channelPointers[i]->isMonitored
      } // end cycling through channels

      // Simple implementation of a "noise gate" on audio output
      expander.process(buffer.getWritePointer(0), // expand the left channel
                         buffer.getNumSamples());

      // copy the signal into the right channel (no stereo audio yet!)
      buffer.addFrom(1,    // destChannel
                     0,  // destSampleOffset
                     buffer,     // source
                      0,    // sourceChannel
                      0,// sourceSampleOffset
                      valuesNeeded,        // number of samples
                      1.0);      // gain to apply to source
    }
  }
}

// ==========================================================

Expander::Expander()
{
  threshold = 1.f;
  output = 1.f;
    
  env = 0.f;
  gain = 1.f;

  setAttack(1.0f);
  setRelease(1.0f);
  setRatio(1.2); // ratio > 1.0 will decrease gain below threshold
}

void Expander::setThreshold(float value)
{
    threshold = value;
    transfer_B = output * pow(threshold, -transfer_A);

    std::cout << "Threshold set to " << threshold << std::endl;
    std::cout << "transfer_B set to " << transfer_B << std::endl;
}


void Expander::setRatio(float value)
{
    transfer_A = value - 1.f;
    transfer_B = output * pow(threshold, -transfer_A);
}


void Expander::setAttack(float value)
{
    attack = exp(-1.f/value);
}


void Expander::setRelease(float value)
{
    release = exp(-1.f/value);
    envelope_decay = exp(-4.f/value); /* = exp(-1/(0.25*value)) */
}


void Expander::process(float* sampleData, int numSamples)
{
  float det, transfer_gain;

  for(int i = 0; i < numSamples; i++)
  {
      det = fabs(sampleData[i]);
      det += 10e-30f; /* add tiny DC offset (-600dB) to prevent denormals */

      env = det >= env ? det : det + envelope_decay*(env-det);

      transfer_gain = env < threshold ? pow(env, transfer_A) * transfer_B : output;

      gain = transfer_gain < gain ?
                      transfer_gain + attack * (gain - transfer_gain) :
                      transfer_gain + release * (gain - transfer_gain);

      sampleData[i] = sampleData[i] * gain; 
  }
}