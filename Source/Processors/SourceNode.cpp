/*
  ==============================================================================

    SourceNode.cpp
    Created: 7 May 2011 5:07:14pm
    Author:  jsiegle

  ==============================================================================
*/


#include "SourceNode.h"
#include "Editors/SourceNodeEditor.h"
#include <stdio.h>

SourceNode::SourceNode(const String& name_)
	: GenericProcessor(name_),
	  dataThread(0)
{
	if (getName().equalsIgnoreCase("Intan Demo Board")) {
		setNumOutputs(16);
		setNumInputs(0);
	} else if (getName().equalsIgnoreCase("Custom FPGA")) {
		setNumOutputs(32);
		setNumInputs(0);
	} else if (getName().equalsIgnoreCase("File Reader")) {
		setNumOutputs(16);
		setNumInputs(0);
	}

	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

	//sendActionMessage("Intan Demo Board source created.");
	//sendMessage("Intan Demo Board source created.");

}

SourceNode::~SourceNode() 
{
	config->removeDataSource(this);	
}

float SourceNode::getSampleRate()
{
	if (getName().equalsIgnoreCase("Intan Demo Board")) {
		return 25000.0;
	} else if (getName().equalsIgnoreCase("Custom FPGA")) {
		return 25000.0;
	} else if (getName().equalsIgnoreCase("File Reader")) {
		return 40000.0;
	} else {
		return 44100.0;
	}

}

// void SourceNode::setName(const String name_)
// {
// 	name = name_;

// 	// Source node type determines configuration info

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

void SourceNode::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{
	//
	// We take care of thread creation and destruction in separate enable/disable function
	// 
	// prepareToPlay is called whenever the graph is edited, not only when callbacks are
	// 	about to begin
	//
}

void SourceNode::releaseResources() {}


AudioProcessorEditor* SourceNode::createEditor()
{
	SourceNodeEditor* ed = new SourceNodeEditor(this, viewport);
	setEditor(ed);
	
	std::cout << "Creating editor." << std::endl;
	//filterEditor = new FilterEditor(this);
	return ed;

	//return 0;
}

// void SourceNode::setSourceNode(GenericProcessor* sn) 
// {
// 	sourceNode = 0;
// }

// void SourceNode::setDestNode(GenericProcessor* dn)
// {
// 	destNode = dn;
// 	if (dn != 0)
// 		dn->setSourceNode(this);
// }

//void SourceNode::createEditor() {
	
//}

bool SourceNode::enable() {
	
	std::cout << "Source node received enable signal" << std::endl;

	bool return_code = true;

	if (getName().equalsIgnoreCase("Intan Demo Board")) {
		
		dataThread = new IntanThread();
		inputBuffer = dataThread->getBufferAddress();
		return_code = dataThread->threadStarted();

		if (!return_code)
			deleteAndZero(dataThread);

	} else if (getName().equalsIgnoreCase("Custom FPGA")) {
		dataThread = new FPGAThread();
		inputBuffer = dataThread->getBufferAddress();
	} else if (getName().equalsIgnoreCase("File Reader")) {
		dataThread = new FileReaderThread();
		inputBuffer = dataThread->getBufferAddress();
	}

	return return_code;

}

bool SourceNode::disable() {
	
	std::cout << "Source node received disable signal" << std::endl;

	if (dataThread != 0) {
		delete dataThread;
		dataThread = 0;
	}

	return true;
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
}



