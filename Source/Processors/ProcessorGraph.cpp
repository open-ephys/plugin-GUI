/*
  ==============================================================================

    ProcessorGraph.cpp
    Created: 30 Apr 2011 8:36:35pm
    Author:  jsiegle

  ==============================================================================
*/

#include <stdio.h>

#include "ProcessorGraph.h"

#include "AudioNode.h"
#include "LfpDisplayNode.h"
#include "EventNode.h"
#include "FileReader.h"
#include "FilterNode.h"
#include "GenericProcessor.h"
#include "RecordNode.h"
#include "ResamplingNode.h"
#include "SignalGenerator.h"
#include "SourceNode.h"
#include "SpikeDetector.h"
#include "Utilities/Splitter.h"
#include "../UI/UIComponent.h"
#include "../UI/Configuration.h"
#include "../UI/FilterViewport.h"

ProcessorGraph::ProcessorGraph() : 
	currentNodeId(100),
	RECORD_NODE_ID(199), 
	AUDIO_NODE_ID(200), 
	OUTPUT_NODE_ID(201), 
	RESAMPLING_NODE_ID(202),
	totalAudioConnections(0),
	totalRecordConnections(0)
	
	{

	// ProcessorGraph will always have 0 inputs (all content is generated within graph)
	// but it will have N outputs, where N is the number of channels for the audio monitor
	setPlayConfigDetails(0, // number of inputs
				         2, // number of outputs
				         44100.0, // sampleRate
				         128);    // blockSize

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
		
		processor->setFilterViewport(filterViewport);
		processor->setConfiguration(config);
		processor->addActionListener(messageCenter);

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

	// std::cout << "Clearing nodes..." << std::endl;

	// for (int i = 0; i < getNumNodes(); i++)
	// {
	// 	Node* node = getNode(i);

	// 	int id = node->nodeId;

	// 	if (!(id == RECORD_NODE_ID || id == AUDIO_NODE_ID ||
	// 	      id == OUTPUT_NODE_ID || id == RESAMPLING_NODE_ID))
	// 	{
	// 		   removeNode(id);
	// 	}

	// }

	// std::cout << "Remaining nodes: " << std::endl;
	// for (int i = 0; i < getNumNodes(); i++)
	// {
	// 	Node* node = getNode(i);
	// 	std::cout << "  " << node->getProcessor()->getName() << std::endl;
	// }

	// std::cout << std::endl;
}

void ProcessorGraph::updateConnections(Array<SignalChainTabButton*, CriticalSection> tabs)
{
	clearConnections(); // clear processor graph
	//createDefaultNodes(); // add audio and record nodes
	std::cout << "Updating connections:" << std::endl;

 	for (int n = 0; n < tabs.size(); n++)
	{
		std::cout << "Signal chain " << n << std::endl;
		//if (tabs[n]->hasNewConnections())
		//{

		GenericEditor* sourceEditor = (GenericEditor*) tabs[n]->getEditor();
		GenericProcessor* source = (GenericProcessor*) sourceEditor->getProcessor();

		while (source != 0)// && destEditor->isEnabled())
		{
			std::cout << "Source node: " << source->getName() << ", ";
			GenericProcessor* dest = (GenericProcessor*) source->getDestNode();
			if (dest != 0)
			{
				std::cout << "Dest node: " << dest->getName() << std::endl;
			} else {
				std::cout << "no dest node." << std::endl;
			}

			if (source->enabledState())
			{

				// add the source to the graph if it doesn't already exist
				//Node* node = getNodeForId(source->getNodeId());
				//if (node == 0)
				//	addNode(source, source->getNodeId());

				// add the connections to audio and record nodes if necessary
				if (!(source->isSink() || source->isSource() ||
				      source->isSplitter() || source->isMerger()))
				{
					std::cout << "   Connecting to audio and record nodes." << std::endl;

					for (int chan = 0; chan < source->getNumOutputs(); chan++) {

						addConnection(source->getNodeId(), // sourceNodeID
						  	chan, // sourceNodeChannelIndex
						   	AUDIO_NODE_ID, // destNodeID
						  	getNextFreeAudioChannel()); // destNodeChannelIndex

						addConnection(source->getNodeId(), // sourceNodeID
						  	chan, // sourceNodeChannelIndex
						   	RECORD_NODE_ID, // destNodeID
						  	getNextFreeRecordChannel()); // destNodeChannelIndex
					}
				}

				if (dest != 0) {

					if (dest->enabledState())
						std::cout << "OK." << std::endl;
					else
						std::cout << "Not OK." << std::endl;

					if (dest->enabledState())
					{

						// add dest node to graph if it doesn't already exist
						//node = getNodeForId(dest->getNodeId());
						//if (node == 0)
						//	addNode(dest, dest->getNodeId());

						std::cout << "     Connecting " << source->getName() << " channel ";

						for (int chan = 0; chan < source->getNumOutputs(); chan++) 
						{

							// eventually need to account for splitter and mergers
							
							           std::cout << chan << " ";
							           

							addConnection(source->getNodeId(), // sourceNodeID
							  	chan, // sourceNodeChannelIndex
							   	dest->getNodeId(), // destNodeID
							  	chan); // destNodeChannelIndex
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
			
			source = dest; // switch source and dest
		} // end while source != 0
	} // end "tabs" for loop
} // end method

int ProcessorGraph::getNextFreeAudioChannel()
{
	return totalAudioConnections++;
}

int ProcessorGraph::getNextFreeRecordChannel()
{
	return totalRecordConnections++;
}

GenericProcessor* ProcessorGraph::createProcessorFromDescription(String& description)
{
	int splitPoint = description.indexOf("/");
	String processorType = description.substring(0,splitPoint);
	String subProcessorType = description.substring(splitPoint+1);

	std::cout << processorType << "::" << subProcessorType << std::endl;

	GenericProcessor* processor = 0;

	if (processorType.equalsIgnoreCase("Sources")) {

		if (subProcessorType.equalsIgnoreCase("Intan Demo Board")) {
			processor = new SourceNode(subProcessorType);
			std::cout << "Creating a new data source." << std::endl;
		} else if (subProcessorType.equalsIgnoreCase("Signal Generator"))
		{
			processor = new SignalGenerator();
			std::cout << "Creating a new signal generator." << std::endl;
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

	 	}

	} else if (processorType.equalsIgnoreCase("Sinks")) {

		//if (subProcessorType.equalsIgnoreCase("Stream Viewer")) {
			
		if (subProcessorType.equalsIgnoreCase("LFP Viewer")) {
			std::cout << "Creating a display node." << std::endl;
			processor = new LfpDisplayNode();
		   
		    std::cout << "Graph data viewport: " << UI->getDataViewport() << std::endl;
			 processor->setDataViewport(UI->getDataViewport());
			processor->setUIComponent(UI);
		}
		//}
	
		sendActionMessage("New visualizer created.");
	}

	return processor;
}


void ProcessorGraph::removeProcessor(GenericProcessor* processor) {
	
	std::cout << "Removing processor with ID " << processor->getNodeId() << std::endl;

	// GenericProcessor* source = processor->getSourceNode();
	// GenericProcessor* dest = processor->getDestNode();
	// int numInputs = processor->getNumInputs();

	// std::cout << "  Source " << source << std::endl;
	// std::cout << "  Dest " << dest << std::endl;

	removeNode(processor->getNodeId());

	// eliminate connections for now
	/*if (dest !=0 && source !=0) {

		std::cout << "   Making new connections...." << std::endl;

		// connect source and dest
		for (int chan = 0; chan < numInputs; chan++) {
			
			addConnection(source->getNodeId(),
						  chan,
						  dest->getNodeId(),
						  chan);
		}

	}*/

	// if (dest != 0)
	// 	dest->setSourceNode(source);
	
	// if (source != 0)
	// 	source->setDestNode(dest);

}

void ProcessorGraph::setUIComponent(UIComponent* ui)
{
	UI = ui;
	//config = ui->getConfiguration();
}

void ProcessorGraph::setFilterViewport(FilterViewport* fv)
{
	filterViewport = fv;
}

void ProcessorGraph::setMessageCenter(MessageCenter* mc)
{
	messageCenter = mc;
}

void ProcessorGraph::setConfiguration(Configuration* c)
{
	config = c;
}


bool ProcessorGraph::enableProcessors() {

	updateConnections(filterViewport->requestSignalChain());

	std::cout << "Enabling processors..." << std::endl;

	bool allClear;

	for (int i = 0; i < getNumNodes(); i++)
	{

		Node* node = getNode(i);

		if (node->nodeId != OUTPUT_NODE_ID)
		{
			GenericProcessor* p = (GenericProcessor*) node->getProcessor();
			allClear = p->enable();

			if (!allClear) {
				sendActionMessage("Could not initialize acquisition. Is the Intan Board plugged in?");
				return false;
			}
		}
	}
	
	filterViewport->signalChainCanBeEdited(false);

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
			allClear = p->disable();

			if (!allClear) {
				sendActionMessage("Could not stop acquisition.");
				return false;
			}
		}
	}

	filterViewport->signalChainCanBeEdited(true);

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

GenericProcessor* ProcessorGraph::getSourceNode(int snID) {

	std::cout << "Requested ID: " << snID << std::endl;
	//if (snID != 0) {
	Node* node = getNodeForId(snID);

	if (node != 0) {
		return (GenericProcessor*) node->getProcessor();
	} else {
		return 0;
	}

}

void ProcessorGraph::saveState()
{
	File file = File("./savedState.xml");
	filterViewport->saveState(file);
}

void ProcessorGraph::loadState()
{
	File file = File("./savedState.xml");
	filterViewport->loadState(file);
}