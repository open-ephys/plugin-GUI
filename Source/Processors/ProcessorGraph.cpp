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
#include "ReferenceNode.h"
#include "AudioResamplingNode.h"
#include "SignalGenerator.h"
#include "SourceNode.h"
#include "EventDetector.h"
#include "SpikeDetector.h"
#include "PhaseDetector.h"
#include "WiFiOutput.h"
#include "ArduinoOutput.h"
#include "FPGAOutput.h"
#include "Utilities/RecordControl.h"
#include "Utilities/Splitter.h"
#include "Utilities/Merger.h"
#include "../UI/UIComponent.h"
#include "../UI/EditorViewport.h"

ProcessorGraph::ProcessorGraph() : currentNodeId(100)	
	{

	// The ProcessorGraph will always have 0 inputs (all content is generated within graph)
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

	// add audio node -- takes all inputs and selects those to be used for audio monitoring
	AudioNode* an = new AudioNode();
	an->setNodeId(AUDIO_NODE_ID);

	// add audio resampling node -- resamples continuous signals to 44.1kHz
	AudioResamplingNode* arn = new AudioResamplingNode();
	arn->setNodeId(RESAMPLING_NODE_ID);

	addNode(on, OUTPUT_NODE_ID);
	addNode(recn, RECORD_NODE_ID);
	addNode(an, AUDIO_NODE_ID);
	addNode(arn, RESAMPLING_NODE_ID);

	// connect audio subnetwork
	for (int n = 0; n < 2; n++) {
		
		addConnection(AUDIO_NODE_ID, n,
		              RESAMPLING_NODE_ID, n);
		
		addConnection(RESAMPLING_NODE_ID, n,
		              OUTPUT_NODE_ID, n);

	}

	addConnection(AUDIO_NODE_ID, midiChannelIndex,
		              RESAMPLING_NODE_ID, midiChannelIndex);

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

void ProcessorGraph::clearSignalChain()
{

	int n = 0;

	while (getNumNodes() > 4) 
	{
		 Node* node = getNode(n);
		 int nodeId = node->nodeId;

		 if (nodeId != OUTPUT_NODE_ID && 
		 	 nodeId != AUDIO_NODE_ID &&
		 	 nodeId != RECORD_NODE_ID &&
		 	 nodeId != RESAMPLING_NODE_ID)
		 {
			 GenericProcessor* p =(GenericProcessor*) node->getProcessor();
			 removeProcessor(p);
		 } else {
		 	n++;
		 }
	}

}

void ProcessorGraph::changeListenerCallback(ChangeBroadcaster* source)
{

	ColourSelector* cs = dynamic_cast <ColourSelector*> (source);

	for (int i = 0; i < getNumNodes(); i++)
	{
		 Node* node = getNode(i);

		 int nodeId = node->nodeId;

		 if (nodeId != OUTPUT_NODE_ID && 
		 	 nodeId != AUDIO_NODE_ID &&
		 	 nodeId != RECORD_NODE_ID &&
		 	 nodeId != RESAMPLING_NODE_ID)
		 {
			 GenericProcessor* p =(GenericProcessor*) node->getProcessor();
			 GenericEditor* e = (GenericEditor*) p->getEditor();
			 e->refreshColors();
		 } 
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

					//source->setStartChannel(getAudioNode()->getNextChannel(false));


					for (int chan = 0; chan < source->getNumOutputs(); chan++) {

						getAudioNode()->addInputChannel(source, chan);

						// std::cout << "Connecting to audio channel: " << 
						// 	      getAudioNode()->getNextChannel(false) << std::endl;

						//getAudioNode()->enableCurrentChannel(source->audioStatus(chan));
				    
						addConnection(source->getNodeId(), 		   // sourceNodeID
						  	chan, 						           // sourceNodeChannelIndex
						   	AUDIO_NODE_ID, 					       // destNodeID
						  	getAudioNode()->getNextChannel(true)); // destNodeChannelIndex
									// add 2 to account for 2 output channels

						
						//std::cout << getAudioNode()->getNextChannel(false) << " ";

						getRecordNode()->addInputChannel(source, chan);

						// std::cout << "Connecting to record channel: " << 
						// 	      getRecordNode()->getNextChannel(false) << std::endl;


						addConnection(source->getNodeId(),          // sourceNodeID
						  	chan,                                   // sourceNodeChannelIndex
						   	RECORD_NODE_ID, 					    // destNodeID
						  	getRecordNode()->getNextChannel(true)); // destNodeChannelIndex
					
					}

					// connect event channel
					addConnection(source->getNodeId(), 				// sourceNodeID
					  	midiChannelIndex, 							// sourceNodeChannelIndex
					   	RECORD_NODE_ID, 							// destNodeID
					  	midiChannelIndex);							// destNodeChannelIndex

					// connect event channel
					addConnection(source->getNodeId(), 				// sourceNodeID
					  	midiChannelIndex, 							// sourceNodeChannelIndex
					   	AUDIO_NODE_ID, 							// destNodeID
					  	midiChannelIndex);							// destNodeChannelIndex


					getRecordNode()->addInputChannel(source, midiChannelIndex);

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

						for (int chan = 0; chan < source->getNumOutputs(); chan++) 
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


GenericProcessor* ProcessorGraph::createProcessorFromDescription(String& description)
{
	int splitPoint = description.indexOf("/");
	String processorType = description.substring(0,splitPoint);
	String subProcessorType = description.substring(splitPoint+1);

	std::cout << processorType << "::" << subProcessorType << std::endl;

	GenericProcessor* processor = 0;

	if (processorType.equalsIgnoreCase("Sources")) {

		if (subProcessorType.equalsIgnoreCase("Intan Demo Board") || 
			subProcessorType.equalsIgnoreCase("File Reader") || 
			subProcessorType.equalsIgnoreCase("Custom FPGA")) {
			
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

		
		//sendActionMessage("New source node created.");
		

	} else if (processorType.equalsIgnoreCase("Filters")) {

		if (subProcessorType.equalsIgnoreCase("Bandpass Filter")) {

			std::cout << "Creating a new filter." << std::endl;
			processor = new FilterNode();

		} else if (subProcessorType.equalsIgnoreCase("Resampler")) {
			std::cout << "Creating a new resampler." << std::endl;
			processor = new ResamplingNode();
		
		} else if (subProcessorType.equalsIgnoreCase("Spike Detector")) {
			std::cout << "Creating a new spike detector." << std::endl;
			processor = new SpikeDetector();
		}
		else if (subProcessorType.equalsIgnoreCase("Event Detector")) {
			std::cout << "Creating a new event detector." << std::endl;
			processor = new EventDetector();
		}
		 else if (subProcessorType.equalsIgnoreCase("Phase Detector")) {
			std::cout << "Creating a new phase detector." << std::endl;
			processor = new PhaseDetector();
		}
		else if (subProcessorType.equalsIgnoreCase("Digital Reference")) {
			std::cout << "Creating a new digital reference." << std::endl;
			processor = new ReferenceNode();
		}

		//sendActionMessage("New filter node created.");

	} else if (processorType.equalsIgnoreCase("Utilities")) {

	 	if (subProcessorType.equalsIgnoreCase("Splitter")) {
			
			std::cout << "Creating a new splitter." << std::endl;
			processor = new Splitter();

			//sendActionMessage("New splitter created.");

	 	} else if (subProcessorType.equalsIgnoreCase("Merger")) {
	 		
	 		std::cout << "Creating a new merger." << std::endl;
			processor = new Merger();

			//sendActionMessage("New merger created.");

		} else if (subProcessorType.equalsIgnoreCase("Record Controller")) {

			std::cout << "Creating a new record controller." << std::endl;
			processor = new RecordControl();

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
		else if (subProcessorType.equalsIgnoreCase("Arduino Output")) {
			std::cout << "Creating an Arduino node." << std::endl;
			processor = new ArduinoOutput();
		}
		else if (subProcessorType.equalsIgnoreCase("FPGA Output")) {
			std::cout << "Creating an FPGA output node." << std::endl;
			processor = new FPGAOutput();
		}
	
		//sendActionMessage("New sink created.");
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
			//	sendActionMessage("Could not initialize acquisition.");
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
			p->enableEditor();
			p->enable();
		}
	}
	
	getEditorViewport()->signalChainCanBeEdited(false);

//	sendActionMessage("Acquisition started.");

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
			p->disableEditor();
			allClear = p->disable();

			if (!allClear) {
			//	sendActionMessage("Could not stop acquisition.");
				return false;
			}
		}
	}

	getEditorViewport()->signalChainCanBeEdited(true);

//	sendActionMessage("Acquisition ended.");

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

// void ProcessorGraph::saveState()
// {
// 	File file = File("./savedState.xml");
// 	getEditorViewport()->saveState(file);
// }

// void ProcessorGraph::loadState()
// {
// 	File file = File("./savedState.xml");
// 	getEditorViewport()->loadState(file);
// }

