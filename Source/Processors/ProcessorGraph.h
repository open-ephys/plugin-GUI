/*
  ==============================================================================

    ProcessorGraph.h
    Created: 30 Apr 2011 8:36:35pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __PROCESSORGRAPH_H_124F8B50__
#define __PROCESSORGRAPH_H_124F8B50__

#include "../../JuceLibraryCode/JuceHeader.h"

class GenericProcessor;
class RecordNode;
class SourceNode;
class FilterViewport;
class SignalChainTabButton;
class AudioNode;
class UIComponent;
class Configuration;
class MessageCenter;

class ProcessorGraph : public AudioProcessorGraph,
					   public ActionBroadcaster
{
public:
	ProcessorGraph();
	~ProcessorGraph();

	void* createNewProcessor(String& description);
	GenericProcessor* createProcessorFromDescription(String& description);

	void removeProcessor(GenericProcessor* processor);

	bool enableProcessors();
	bool disableProcessors();

	RecordNode* getRecordNode();
	GenericProcessor* getSourceNode(int snID);
	AudioNode* getAudioNode();

	void setUIComponent(UIComponent* ui);
	void setFilterViewport(FilterViewport *fv);
	void setMessageCenter(MessageCenter* mc);
	void setConfiguration(Configuration* config);

	void updateConnections(Array<SignalChainTabButton*, CriticalSection>);


	void saveState();
	void loadState();
	//const String saveState(const File& file);
	//const String loadState(const File& file);

	//XmlElement* createNodeXml(GenericProcessor*);

	int getNextFreeAudioChannel();
	int getNextFreeRecordChannel();

private:	

	int currentNodeId;

	Array<int> source_node_IDs;

	const int RECORD_NODE_ID;
	const int AUDIO_NODE_ID;
	const int OUTPUT_NODE_ID;
	const int RESAMPLING_NODE_ID;

	void createDefaultNodes();
	void clearConnections();

	UIComponent* UI;
	FilterViewport* filterViewport;
	Configuration* config;
	MessageCenter* messageCenter;

	int totalAudioConnections;
	int totalRecordConnections;

};



#endif  // __PROCESSORGRAPH_H_124F8B50__
