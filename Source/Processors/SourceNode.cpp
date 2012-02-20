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

#include "SourceNode.h"
#include "Editors/SourceNodeEditor.h"
#include <stdio.h>

SourceNode::SourceNode(const String& name_)
	: GenericProcessor(name_),
	  dataThread(0),
	  sourceCheckInterval(750), wasDisabled(true)
{
	if (getName().equalsIgnoreCase("Intan Demo Board")) {
		dataThread = new IntanThread(this);
	} else if (getName().equalsIgnoreCase("Custom FPGA")) {
		dataThread = new FPGAThread(this);
	} else if (getName().equalsIgnoreCase("File Reader")) {
		dataThread = new FileReaderThread(this);
	}

	setNumInputs(0);

	if (dataThread != 0) {
		setNumOutputs(dataThread->getNumChannels());
		inputBuffer = dataThread->getBufferAddress();
	} else {
		setNumOutputs(10);
	}

	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

	if (dataThread != 0)
	{
		if (!dataThread->foundInputSource())
		{
			enabledState(false);
		}
	} else {
		enabledState(false);
	}

	// check for input source every two seconds
	startTimer(sourceCheckInterval); 

}

SourceNode::~SourceNode() 
{
	if (dataThread != 0)
		deleteAndZero(dataThread);

	config->removeDataSource(this);	
}

float SourceNode::getSampleRate()
{

	if (dataThread != 0)
		return dataThread->getSampleRate();
	else
		return 44100.0;
}

void SourceNode::enabledState(bool t)
{
	if (t && !dataThread->foundInputSource())
	{
		isEnabled = false;
	} else {
		isEnabled = t;
	}


}

void SourceNode::setConfiguration(Configuration* cf)
{
	config = cf;

     DataSource* d = new DataSource(this, config);

  //   // add tetrodes -- should really be doing this dynamically
     d->addTrode(4, "TT1");
     d->addTrode(4, "TT2");
     d->addTrode(4, "TT3");
     d->addTrode(4, "TT4");

     for (int n = 0; n < d->numTetrodes(); n++)
      {
           std::cout << d->getTetrode(n)->getName();
      }
      std::cout << std::endl;

	 // // add a new data source to this configuration
     config->addDataSource(d);

}


void SourceNode::setParameter (int parameterIndex, float newValue)
{
	//std::cout << "Got parameter change notification";
}

AudioProcessorEditor* SourceNode::createEditor()
{
	SourceNodeEditor* ed = new SourceNodeEditor(this, viewport);
	setEditor(ed);
	
	std::cout << "Creating editor." << std::endl;
	return ed;
}

void SourceNode::timerCallback()
{
	if (dataThread->foundInputSource() && !isEnabled)
	{
		std::cout << "Input source found." << std::endl;
		//stopTimer(); // check for input source every two seconds
		enabledState(true);
		GenericEditor* ed = (GenericEditor*) getEditor();
		viewport->updateVisibleEditors(ed, 4);
	} else if (!dataThread->foundInputSource() && isEnabled) {
		std::cout << "No input source found." << std::endl;
		enabledState(false);
		GenericEditor* ed = (GenericEditor*) getEditor();
		viewport->updateVisibleEditors(ed, 4);
	}
}

bool SourceNode::enable() {
	
	std::cout << "Source node received enable signal" << std::endl;

	wasDisabled = false;

	if (dataThread != 0)
	{
		if (dataThread->foundInputSource())
		{
			dataThread->startAcquisition();
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}

	stopTimer();

	

	// bool return_code = true;

	// if (getName().equalsIgnoreCase("Intan Demo Board")) {
		
	// 	dataThread = new IntanThread();
	// 	inputBuffer = dataThread->getBufferAddress();
	// 	return_code = dataThread->threadStarted();

	// 	if (!return_code)
	// 		deleteAndZero(dataThread);

	// } else if (getName().equalsIgnoreCase("Custom FPGA")) {
	// 	dataThread = new FPGAThread();
	// 	inputBuffer = dataThread->getBufferAddress();
	// } else if (getName().equalsIgnoreCase("File Reader")) {
	// 	dataThread = new FileReaderThread();
	// 	inputBuffer = dataThread->getBufferAddress();
	// }

	// return return_code;

}

bool SourceNode::disable() {

	std::cout << "Source node received disable signal" << std::endl;

	if (dataThread != 0)
		dataThread->stopAcquisition();
	
	startTimer(2000);

	wasDisabled = true;

	return true;
}

void SourceNode::acquisitionStopped()
{
	//if (!dataThread->foundInputSource()) {
		
		if (!wasDisabled) {
			std::cout << "Source node sending signal to UI." << std::endl;
			UI->disableCallbacks();
			enabledState(false);
			GenericEditor* ed = (GenericEditor*) getEditor();
			viewport->updateVisibleEditors(ed, 4);
		}
	//}
}


void SourceNode::process(AudioSampleBuffer &outputBuffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{

	//std::cout << "Source node processing." << std::endl;
	//std::cout << outputBuffer.getNumChannels() << " " << outputBuffer.getNumSamples() << std::endl;

	
	 outputBuffer.clear();
	 nSamples = inputBuffer->readAllFromBuffer(outputBuffer,outputBuffer.getNumSamples());
	// //setNumSamples(numRead); // write the total number of samples
	// setNumSamples(midiMessages, numRead);
	//std::cout << numRead << std::endl;

	//addMidiEvent(midiMessages,10, 10);
}



