/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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


#include "AudioNode.h"

AudioNode::AudioNode()
	: GenericProcessor("Audio Node"), volume(0.00001f), audioEditor(0)
{

	settings.numInputs = 64;
	settings.numOutputs = 2;

	// 64 inputs, 2 outputs (left and right channel)
	setPlayConfigDetails(getNumInputs(),getNumOutputs(),44100.0,128);

	leftChan.add(0);
	rightChan.add(1);

	currentChannel = 0;


}


AudioNode::~AudioNode() {



}

AudioProcessorEditor* AudioNode::createEditor()
{
	
	audioEditor = new AudioEditor(this);

	//setEditor(editor);
	
	return audioEditor; 

}


void AudioNode::setParameter (int parameterIndex, float newValue)
{
	// change left channel, right channel, or volume
	if (parameterIndex == 1) // volume level
		volume = newValue*0.00001f;

}

void AudioNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{

	//std::cout << "Audio node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;


	// if (currentChannel == 0)
	// {
	// 	buffer.clear(1,0,buffer.getNumSamples());
	// 	buffer.copyFrom();
	// } else if (currentChannel == 1)
	// {
	// 	buffer.clear(0,0,buffer.getNumSamples());
	// 	buffer.copyFrom();
	// } else {

	// currently monitoring channels 2 and 3 ONLY!

	buffer.clear(0,0,buffer.getNumSamples());
	buffer.clear(1,0,buffer.getNumSamples());

	for (int n = 0; n < leftChan.size(); n++) {
		buffer.addFrom(0,  // destination channel
					   0,  // destination start sample
					   buffer,      // source
					   leftChan[n]+2, // source channel
					   0,           // source start sample
					   buffer.getNumSamples(), //  number of samples
					   volume       // gain to apply
					   );
	}
	
	for (int n = 0; n < rightChan.size(); n++) {
		buffer.addFrom(1,  // destination channel
					   0,  // destination start sample
					   buffer,      // source
					   rightChan[n]+2, // source channel
					   0,           // source start sample
					   buffer.getNumSamples(), //  number of samples
					   volume       // gain to apply
					   );
	}
	//}
}
