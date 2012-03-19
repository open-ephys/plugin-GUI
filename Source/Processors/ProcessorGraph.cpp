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

#include <stdio.h>

#include "ProcessorGraph.h"

#include "AudioNode.h"
#include "LfpDisplayNode.h"
#include "SpikeDisplayNode.h"
#include "EventNode.h"
#include "FilterNode.h"
#include "GenericProcessor.h"
#include "RecordNode.h"
#include "ResamplingNode.h"
#include "SignalGenerator.h"
#include "SourceNode.h"
#include "SpikeDetector.h"
#include "WiFiOutput.h"
#include "Utilities/Splitter.h"
#include "Utilities/Merger.h"
#include "../UI/UIComponent.h"
#include "../UI/Configuration.h"
#include "../UI/EditorViewport.h"

ProcessorGraph::ProcessorGraph() : 
	currentNodeId(100),
	RECORD_NODE_ID(199), 
	AUDIO_NODE_ID(200), 
	OUTPUT_NODE_ID(201), 
	RESAMPLING_NODE_ID(202)
	//totalAudioConnections(0),
	//totalRecordConnections(0)
	
	{

	// ProcessorGraph will always have 0 inputs (all content is generated within graph)
	// but it will have N outputs, where N is the number of channels for the audio monitor
	setPlayConfigDetails(0, // number of inputs
				         2, // number of outputs
				         44100.0, // sampleRate
				         1024);    // blockSize

	createDefaultNodes();

}

ProcessorGraph::~ProcessorGraph() { }


void ProcessorGraph::createDefaultNodes()
{

	// add output node -- sends output to the audio card
	AudioProcessorGraph::AudioGraphIOProcessor* on = 
		new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);

	// add record node -- sends output to disk
	RecordNode* recn = new RecordNode();
	recn->setNodeId(RECORD_NODE_ID);
	//recn->setConfiguration(config);

	// add audio node -- takes all inputs and selects those to be used for audio monitoring
	AudioNode* an = new AudioNode();
	recn->setNodeId(AUDIO_NODE_ID);
	//an->setConfiguration(config);

	// add resampling node -- resamples continuous signals to 44.1kHz
	ResamplingNode* rn = new ResamplingNode(true);
	rn->setNodeId(RESAMPLING_NODE_ID);

	addNode(on,OUTPUT_NODE_ID);
	addNode(recn,RECORD_NODE_ID);
	addNode(an, AUDIO_NODE_ID);
	addNode(rn, RESAMPLING_NODE_ID);

	// connect audio network
	for (int n = 0; n < 2; n++) {
		
		addConnection(AUDIO_NODE_ID, n,
		              RESAMPLING_NODE_ID, n);
		
		addConnection(RESAMPLING_NODE_ID, n,
		              OUTPUT_NODE_ID, n);

	}

	std::cout << "Default nodes created." << std::endl;
	
}

void* ProcessorGraph::createNewProcessor(String& description)//,
										// GenericProcessor* source,
										// GenericProcessor* dest) 
{

	GenericProcessor* processor = createProcessorFromDescription(description);

	int id = currentNodeId++;

	if (processor != 0) {

		processor->setNodeId(id); // identifier within processor graph
		std::cout << "  Adding node to graph with ID number " << id << std::endl;
		
		processor->setUIComponent(getUIComponent()); // give access to important pointers

		addNode(processor,id); // have to add it so it can be deleted by the graph

		return processor->createEditor();

	} else {

		sendActionMessage("Not a valid processor type.");

		return 0;
	}

}

void ProcessorGraph::clearConnections()
{

	for (int i = 0; i < getNumConnections(); i++)
	{
		const Connection* connection = getConnection(i);

		if (connection->destNodeId == RESAMPLING_NODE_ID ||
		    connection->destNodeId == OUTPUT_NODE_ID)
		 {
		 	; // leave it   	
		 } else {
		 	removeConnection(i);
		 }
	}


	for (int i = 0; i < getNumNodes(); i++)
	{
		 Node* node = getNode(i);

		 if (node->nodeId != OUTPUT_NODE_ID) {
			 GenericProcessor* p =(GenericProcessor*) node->getProcessor();
			 p->resetConnections();
		}
	}


}

void ProcessorGraph::updateConnections(Array<SignalChainTabButton*, CriticalSection> tabs)
{
	clearConnections(); // clear processor graph

	std::cout << "Updating connections:" << std::endl;

	Array<GenericProcessor*> splitters;

 	for (int n = 0; n < tabs.size(); n++)
	{
		std::cout << "Signal chain " << n << std::endl;

		GenericEditor* sourceEditor = (GenericEditor*) tabs[n]->getEditor();
		GenericProcessor* source = (GenericProcessor*) sourceEditor->getProcessor();

		while (source != 0)// && destEditor->isEnabled())
		{
			std::cout << "Source node: " << source->getName() << ", ";
			GenericProcessor* dest = (GenericProcessor*) source->getDestNode();

			if (dest != 0)
			{
				std::cout << "Dest node: " << dest->getName() << std::endl;
				if (dest->isMerger()) // move it forward by one
				{
					dest = dest->getDestNode();
				} else if (dest->isSplitter())
				{
					if (!dest->wasConnected)
						splitters.add(dest);

					dest = dest->getDestNode();
				}

			} else {
				std::cout << "no dest node." << std::endl;
			}

			if (source->enabledState())
			{

				// add the connections to audio and record nodes if necessary


				if (!(source->isSink() ||
				      source->isSplitter() || source->isMerger()) && !(source->wasConnected))
				{
					std::cout << "   Connecting to audio and record nodes." << std::endl;

					for (int chan = 0; chan < source->getNumOutputs(); chan++) {

						addConnection(source->getNodeId(), // sourceNodeID
						  	chan, // sourceNodeChannelIndex
						   	AUDIO_NODE_ID, // destNodeID
						  	getAudioNode()->getNextChannel(true)); // destNodeChannelIndex

						 std::cout << getAudioNode()->getNextChannel(false) << " ";

						addConnection(source->getNodeId(), // sourceNodeID
						  	chan, // sourceNodeChannelIndex
						   	RECORD_NODE_ID, // destNodeID
						  	getRecordNode()->getNextChannel(true)); // destNodeChannelIndex
					}
				}

				std::cout << std::endl;

				if (dest != 0) {

					if (dest->enabledState())
						std::cout << "     OK." << std::endl;
					else 
						std::cout << "     Not OK." << std::endl;

					if (dest->enabledState())
					{

						std::cout << "     Connecting " << source->getName() << " channel ";

						//int nextChan;
						//int chan = 0;

						for (int chan = 0; chan < source->getNumOutputs(); chan++) 
						
						//ile ((nextChan = dest->getNextChannel(true)) != -1)
						{
							std::cout << chan << " ";
							           
							addConnection(source->getNodeId(), // sourceNodeID
							  	chan, // sourceNodeChannelIndex
							   	dest->getNodeId(), // destNodeID
							  	dest->getNextChannel(true)); // destNodeChannelIndex
						}

						std::cout << " to " << dest->getName() << std::endl;
							
							std::cout << "     Connecting " << source->getName() <<
							           " event channel to " <<
							           dest->getName() << std::endl;
							// connect event channel
							addConnection(source->getNodeId(), // sourceNodeID
							  	midiChannelIndex, // sourceNodeChannelIndex
							   	dest->getNodeId(), // destNodeID
							  	midiChannelIndex); // destNodeChannelIndex

					}

				}
			}	
			
			source->wasConnected = true;
			source = dest; // switch source and dest

			if (source == 0 && splitters.size() > 0)
			{
				dest = splitters.getFirst(); // dest is now the splitter
				splitters.remove(0); // take it out of the 
				dest->switchIO(); // switch to the other destination
				dest->wasConnected = true; // don't want to re-add splitter
				source = dest->getSourceNode(); // splitter is now source
			}

		} // end while source != 0
	} // end "tabs" for loop
} // end method

// int ProcessorGraph::getNextFreeAudioChannel()
// {
// 	return totalAudioConnections++;
// }

// int ProcessorGraph::getNextFreeRecordChannel()
// {
// 	return totalRecordConnections++;
// }

GenericProcessor* ProcessorGraph::createProcessorFromDescription(String& description)
{
	int splitPoint = description.indexOf("/");
	String processorType = description.substring(0,splitPoint);
	String subProcessorType = description.substring(splitPoint+1);

	std::cout << processorType << "::" << subProcessorType << std::endl;

	GenericProcessor* processor = 0;

	if (processorType.equalsIgnoreCase("Sources")) {

		if (subProcessorType.equalsIgnoreCase("Intan Demo Board")) {
			
			// only one Intan Demo Board at a time, please
			if (!processorWithSameNameExists(subProcessorType)) {
				processor = new SourceNode(subProcessorType);
			}
				
			std::cout << "Creating a new data source." << std::endl;
		} else if (subProcessorType.equalsIgnoreCase("Signal Generator"))
		{
			processor = new SignalGenerator();
			std::cout << "Creating a new signal generator." << std::endl;
		} else if (subProcessorType.equalsIgnoreCase("Event Generator"))
		{
			processor = new EventNode();
			std::cout << "Creating a new event node." << std::endl;
		}

		
		sendActionMessage("New source node created.");
		

	} else if (processorType.equalsIgnoreCase("Filters")) {

		if (subProcessorType.equalsIgnoreCase("Bandpass Filter")) {
			std::cout << "Creating a new filter." << std::endl;
			processor = new FilterNode();

		} else if (subProcessorType.equalsIgnoreCase("Resampler")) {
			std::cout << "Creating a new resampler." << std::endl;
			processor = new ResamplingNode(false);
		
		} else if (subProcessorType.equalsIgnoreCase("Spike Detector")) {
			std::cout << "Creating a new spike detector." << std::endl;
			processor = new SpikeDetector();
		}

		sendActionMessage("New filter node created.");

	} else if (processorType.equalsIgnoreCase("Utilities")) {

	 	if (subProcessorType.equalsIgnoreCase("Splitter")) {
			
			std::cout << "Creating a new splitter." << std::endl;
			processor = new Splitter();

			sendActionMessage("New splitter created.");

	 	} else if (subProcessorType.equalsIgnoreCase("Merger")) {
	 		
	 		std::cout << "Creating a new merger." << std::endl;
			processor = new Merger();

			sendActionMessage("New merger created.");

	 	}

	} else if (processorType.equalsIgnoreCase("Sinks")) {

		if (subProcessorType.equalsIgnoreCase("LFP Viewer")) {
			std::cout << "Creating an LfpDisplayNode." << std::endl;
			processor = new LfpDisplayNode();
		   
		   // std::cout << "Graph data viewport: " << UI->getDataViewport() << std::endl;
			// processor->setDataViewport(getDataViewport());
			//processor->setUIComponent(UI);
		} 
		else if (subProcessorType.equalsIgnoreCase("Spike Viewer")) {
			std::cout << "Creating an SpikeDisplayNode." << std::endl;
			processor = new SpikeDisplayNode();	 
		}
		else if (subProcessorType.equalsIgnoreCase("WiFi Output")) {
			std::cout << "Creating a WiFi node." << std::endl;
			processor = new WiFiOutput();
		}
	
		sendActionMessage("New sink created.");
	}

	return processor;
}


bool ProcessorGraph::processorWithSameNameExists(const String& name)
{
	for (int i = 0; i < getNumNodes(); i++)
	{
	 	Node* node = getNode(i);

	 	if (name.equalsIgnoreCase(node->getProcessor()->getName()))
	 		return true;
	
	 }

	 return false;

}


void ProcessorGraph::removeProcessor(GenericProcessor* processor) {
	
	std::cout << "Removing processor with ID " << processor->getNodeId() << std::endl;

	removeNode(processor->getNodeId());

}

// void ProcessorGraph::setUIComponent(UIComponent* ui)
// {
// 	UI = ui;
// }

// void ProcessorGraph::setFilterViewport(FilterViewport* fv)
// {
// 	filterViewport = fv;
// }

// void ProcessorGraph::setMessageCenter(MessageCenter* mc)
// {
// 	messageCenter = mc;
// }

// void ProcessorGraph::setConfiguration(Configuration* c)
// {
// 	config = c;
// }


bool ProcessorGraph::enableProcessors() {

	updateConnections(getEditorViewport()->requestSignalChain());

	std::cout << "Enabling processors..." << std::endl;

	bool allClear;

	if (getNumNodes() < 5)
	{
		getUIComponent()->disableCallbacks();
		return false;
	}

	for (int i = 0; i < getNumNodes(); i++)
	{

		Node* node = getNode(i);

		if (node->nodeId != OUTPUT_NODE_ID)
		{
			GenericProcessor* p = (GenericProcessor*) node->getProcessor();
			allClear = p->isReady();

			if (!allClear) {
				std::cout << p->getName() << " said it's not OK." << std::endl;
				sendActionMessage("Could not initialize acquisition.");
				getUIComponent()->disableCallbacks();
				return false;

			}
		}
	}

	for (int i = 0; i < getNumNodes(); i++)
	{

		Node* node = getNode(i);

		if (node->nodeId != OUTPUT_NODE_ID)
		{
			GenericProcessor* p = (GenericProcessor*) node->getProcessor();
			p->enable();
		}
	}
	
	getEditorViewport()->signalChainCanBeEdited(false);

	sendActionMessage("Acquisition started.");

	return true;
}

bool ProcessorGraph::disableProcessors() {

	std::cout << "Disabling processors..." << std::endl;

	bool allClear;

	for (int i = 0; i < getNumNodes(); i++)
	{
		Node* node = getNode(i);
		if (node->nodeId != OUTPUT_NODE_ID)
		{
			GenericProcessor* p = (GenericProcessor*) node->getProcessor();
			std::cout << "Disabling " << p->getName() << std::endl;
			allClear = p->disable();

			if (!allClear) {
				sendActionMessage("Could not stop acquisition.");
				return false;
			}
		}
	}

	getEditorViewport()->signalChainCanBeEdited(true);

	sendActionMessage("Acquisition ended.");

	return true;
}


AudioNode* ProcessorGraph::getAudioNode() {
	
	Node* node = getNodeForId(AUDIO_NODE_ID);
	return (AudioNode*) node->getProcessor();

}

RecordNode* ProcessorGraph::getRecordNode() {
	
	Node* node = getNodeForId(RECORD_NODE_ID);
	return (RecordNode*) node->getProcessor();

}

void ProcessorGraph::saveState()
{
	File file = File("./savedState.xml");
	getEditorViewport()->saveState(file);
}

void ProcessorGraph::loadState()
{
	File file = File("./savedState.xml");
	getEditorViewport()->loadState(file);
}