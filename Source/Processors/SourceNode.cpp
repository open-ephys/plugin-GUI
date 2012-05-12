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
	  dataThread(0), inputBuffer(0),
	  sourceCheckInterval(2000), wasDisabled(true)
{

	std::cout << "creating source node." << std::endl;

	if (getName().equalsIgnoreCase("Intan Demo Board")) {
		dataThread = new IntanThread(this);
	} else if (getName().equalsIgnoreCase("Custom FPGA")) {
		dataThread = new FPGAThread(this);//FPGAThread(this);
	} else if (getName().equalsIgnoreCase("File Reader")) {
		dataThread = new FileReaderThread(this);
	}

	if (dataThread != 0)
	{
		if (!dataThread->foundInputSource())
		{
			enabledState(false);
		}
	} else {
		enabledState(false);
	}

	// check for input source every few seconds
	startTimer(sourceCheckInterval); 

}

SourceNode::~SourceNode() 
{
}

void SourceNode::updateSettings()
{
	if (inputBuffer == 0 && dataThread != 0)
	{

		inputBuffer = dataThread->getBufferAddress();
		std::cout << "Input buffer address is " << inputBuffer << std::endl;
	}

}

float SourceNode::getSampleRate()
{

	if (dataThread != 0)
		return dataThread->getSampleRate();
	else
		return 44100.0;
}

float SourceNode::getDefaultSampleRate()
{
	if (dataThread != 0)
		return dataThread->getSampleRate();
	else
		return 44100.0;
}

int SourceNode::getDefaultNumOutputs()
{
	if (dataThread != 0)
		return dataThread->getNumChannels();
	else
		return 0;
}

float SourceNode::getDefaultBitVolts()
{
	if (dataThread != 0)
		return dataThread->getBitVolts();
	else
		return 1.0f;
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

void SourceNode::setParameter (int parameterIndex, float newValue)
{
	//std::cout << "Got parameter change notification";
}

AudioProcessorEditor* SourceNode::createEditor()
{
	editor = new SourceNodeEditor(this);
	return editor;
}

void SourceNode::timerCallback()
{
	if (dataThread->foundInputSource())
	{
		if (!isEnabled) {
			std::cout << "Input source found." << std::endl;
			//stopTimer(); // check for input source every two seconds
			enabledState(true);
			GenericEditor* ed = getEditor();
			getEditorViewport()->makeEditorVisible(ed);
		}
	} else {
		if (isEnabled) {
			std::cout << "No input source found." << std::endl;
			enabledState(false);
			GenericEditor* ed = getEditor();
			getEditorViewport()->makeEditorVisible(ed);
		}
	}
}

bool SourceNode::isReady() {
	
	if (dataThread != 0) {
		return dataThread->foundInputSource();
	} else {
		return false;
	}
}

bool SourceNode::enable() {
	
	std::cout << "Source node received enable signal" << std::endl;

	wasDisabled = false;

	if (dataThread != 0)
	{
		dataThread->startAcquisition();
		return true;
	} else {
		return false;
	}

	stopTimer();

}

bool SourceNode::disable() {

	std::cout << "Source node received disable signal" << std::endl;

	if (dataThread != 0)
		dataThread->stopAcquisition();
	
	startTimer(2000);

	wasDisabled = true;

	std::cout << "SourceNode returning true." << std::endl;

	return true;
}

void SourceNode::acquisitionStopped()
{
	//if (!dataThread->foundInputSource()) {
		
		if (!wasDisabled) {
			std::cout << "Source node sending signal to UI." << std::endl;
			getUIComponent()->disableCallbacks();
			enabledState(false);
			GenericEditor* ed = (GenericEditor*) getEditor();
			getEditorViewport()->makeEditorVisible(ed);
		}
	//}
}


void SourceNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &events,
                            int& nSamples)
{
	
	//std::cout << "SOURCE NODE" << std::endl;

	 buffer.clear();
	 nSamples = inputBuffer->readAllFromBuffer(buffer,buffer.getNumSamples());
	
}



