/*
  ==============================================================================

    ResamplingNode.cpp
    Created: 7 May 2011 5:07:58pm
    Author:  jsiegle

  ==============================================================================
*/

#include "ResamplingNode.h"
#include <stdio.h>

ResamplingNode::ResamplingNode(bool destBufferType)
	: GenericProcessor("Resampling Node"), 
	  ratio (1.0), lastRatio (1.0),
	  destBufferPos(0), destBufferIsTempBuffer(destBufferType),
	  destBufferSampleRate(44100.0), sourceBufferSampleRate(40000.0),
	  destBuffer(0), tempBuffer(0), isTransmitting(false)
	
{

	setNumInputs(2);
	setNumOutputs(2);

		setPlayConfigDetails(2, // number of inputs
				         2, // number of outputs
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

	

	// filter->getKind()
	// filter->getName()
	// filter->getNumParams()
	// filter->getParamInfo()
	// filter->getDefaultParams()
	// filter->getParams()
	// filter->getParam()

	// filter->setParam()
	// filter->findParamId()
	// filter->setParamById()
	// filter->setParams()
	// filter->copyParamsFrom()

	// filter->getPoleZeros()
	// filter->response()
	// filter->getNumChannels()
	// filter->reset()
	// filter->process()

	// Filter families:
	// RBJ: from RBJ cookbook (audio-specific)
	// Butterworth
	// ChebyshevI: ripple in the passband
	// ChebyshevII: ripple in the stopband
	// Elliptic: ripple in passband and stopband
	// Bessel: theoretically with linear phase
	// Legendre: steepest transition and monotonic passband
	// Custom: poles and zeros can be specified directly

	// Filter classes:
	// vary by filter family
}

ResamplingNode::~ResamplingNode()
{
	filter = 0;
	deleteAndZero(destBuffer);
	deleteAndZero(tempBuffer);
	//filterEditor = 0;
}

//AudioProcessorEditor* ResamplingNode::createEditor( )
//{
	//filterEditor = new FilterEditor(this);
	
	//std::cout << "Creating editor." << std::endl;
	//filterEditor = new FilterEditor(this);
	//return filterEditor;

//	return 0;
//}


void ResamplingNode::setParameter (int parameterIndex, float newValue)
{

	switch (parameterIndex) {
		
		case 0: destBufferTimebaseSecs = newValue; break;
		case 1: destBufferWidth = roundToInt(newValue);

	}

	// reset to zero and clear if timebase or width has changed.
	destBufferPos = 0; 
	destBuffer->clear(); 

}


void ResamplingNode::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{

	//std::cout << "ResamplingNode preparing to play." << std::endl;

	if (destBufferIsTempBuffer) {
		destBufferSampleRate = sampleRate_;
		tempBuffer->setSize(getNumInputs(), estimatedSamplesPerBlock);
	}
	else {
		destBufferSampleRate = float(destBufferWidth) / destBufferTimebaseSecs;
		destBuffer->setSize(getNumInputs(), destBufferWidth);
	}

	destBuffer->clear();
	tempBuffer->clear();

	destBufferPos = 0;

	updateFilter();

}

void ResamplingNode::updateFilter() {
	
	double cutoffFreq = (ratio > 1.0) ? 2 * destBufferSampleRate  // downsample
									  : destBufferSampleRate / 2; // upsample

    double sampleFreq = (ratio > 1.0) ? sourceBufferSampleRate // downsample
    								  : destBufferSampleRate;  // upsample
								
	Dsp::Params params;
	params[0] = sampleFreq; // sample rate
	params[1] = cutoffFreq; // cutoff frequency
	params[2] = 1.25; //Q //

	filter->setParams (params);
	
}

void ResamplingNode::releaseResources() 
{	

	// if (destBuffer != 0) {
	// 	deleteAndZero(destBuffer);
	// }
	// if (tempBuffer != 0) {
	// 	deleteAndZero(tempBuffer);
	// }
}

void ResamplingNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{

	//std::cout << "Resampling node sample count: " << buffer.getNumSamples() << std::endl;

	int nSamps = nSamples;
	int valuesNeeded;

    //std::cout << "END OF OLD BUFFER." << std::endl;

	if (destBufferIsTempBuffer) {
		ratio = float(nSamps) / float(buffer.getNumSamples());
		valuesNeeded = tempBuffer->getNumSamples();
	} else {
		ratio = sourceBufferSampleRate / destBufferSampleRate;
		valuesNeeded = (int) buffer.getNumSamples() / ratio;
		//std::cout << std::endl;
		//std::cout << "Ratio: " << ratio << std::endl;
		//std::cout << "Values needed: " << valuesNeeded << std::endl;
	}



	if (lastRatio != ratio) {
		updateFilter();
		lastRatio = ratio;
	}


	if (ratio > 1.0001) {
		// pre-apply filter before downsampling
		filter->process (nSamps, buffer.getArrayOfChannels());
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

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {

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


	if (ratio < 0.9999) {

		filter->process (tempBufferPos, tempBuffer->getArrayOfChannels());
		// apply the filter after upsampling
		///////filter->process (totalSamples, buffer.getArrayOfChannels());
	} else if (ratio <= 1.0001) {
		
		// no resampling is being applied, no need to filter, BUT...
		// keep the filter stoked with samples to avoid discontinuities

	}

	if (destBufferIsTempBuffer) {
    	
    	// copy the temp buffer into the original buffer
    	buffer = *tempBuffer;

    	//buffer.setSize(2,0,true,false,true);

    	//for (int n = 0; n < buffer.getNumSamples(); n+= 10)
    	//std::cout << buffer.getRMSLevel(1,0,buffer.getNumSamples()) << " ";
    
    	//std::cout << "END OF NEW BUFFER." << std::endl;

    } else {

    	//std::cout << "Copying into dest buffer..." << std::endl;
    	
    	// copy the temp buffer into the destination buffer

    	int pos = 0;

    	while (*tempBuffer->getSampleData(0,pos) != 0)
    		pos++;

    	int spaceAvailable = destBufferWidth - destBufferPos;
    	int blockSize1 = (spaceAvailable > pos) ? pos : spaceAvailable;
    	int blockSize2 = (spaceAvailable > pos) ? 0 : (pos - spaceAvailable);

    	for (int channel = 0; channel < destBuffer->getNumChannels(); channel++) {

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

    	//std::cout << "Temp buffer position: " << tempBufferPos << std::endl;
    	//std::cout << "Resampling node value:" << *destBuffer->getSampleData(0,0) << std::endl;

    }


}