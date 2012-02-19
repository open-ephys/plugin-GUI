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

#include "FileReader.h"


FileReader::FileReader()
	: GenericProcessor("File Reader"),
	  sampleRate (40000.0),
	  numChannels(16),
	  samplesPerBlock(1024)
{
	setNumOutputs(numChannels);
	setNumInputs(0);
}

FileReader::~FileReader()
{
}


//AudioProcessorEditor* FileReader::createEditor( )
//{
	//filterEditor = new FilterEditor(this);
	
//	std::cout << "Creating editor." << std::endl;
//	sourceEditor = new SourceNodeEditor(this);
//	return sourceEditor;
//}

//AudioProcessorEditor* FilterNode::createEditor(AudioProcessorEditor* const editor)
//{
	
//	return editor;
//}
void FileReader::setParameter (int parameterIndex, float newValue)
{
	//std::cout << "Message received." << std::endl;


}

void FileReader::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{

	samplesPerBlock = int(float(estimatedSamplesPerBlock)/sampleRate*sampleRate);	

	std::cout << "Samples per block = " << samplesPerBlock << std::endl;

	
}

bool FileReader::enable () {

	File file = File("./data_stream_16ch");
	input = file.createInputStream();

	
	std::cout << "File Reader received enable signal." << std::endl;

	return true;

}

bool FileReader::disable() {

	deleteAndZero(input);
	
	std::cout << "File reader received disable signal." << std::endl;

	return true;

}

void FileReader::releaseResources() 
{	

}

void FileReader::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{

	nSamples = samplesPerBlock;
	//std::cout << buffer.getNumChannels() << std::endl;
	
    for (int i = 0; i < samplesPerBlock; ++i)
    {

    	for (int j = 0; j < numChannels; j++) {
    		
    		if (input->isExhausted())
    			input->setPosition(0);   			
    	
    		const float sample = float(input->readShort());

    		*buffer.getSampleData (j, i) = sample;
    	}

    }
}