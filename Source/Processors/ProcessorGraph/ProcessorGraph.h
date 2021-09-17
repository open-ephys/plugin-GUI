/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef __PROCESSORGRAPH_H_124F8B50__
#define __PROCESSORGRAPH_H_124F8B50__

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../../AccessClass.h"

class GenericProcessor;
class GenericEditor;
class RecordNode;
class AudioNode;
class MessageCenter;
class SignalChainTabButton;
class TimestampSourceSelectionWindow;

struct ProcessorDescription {
    bool fromProcessorList;
    String processorName;
    int processorType;
    int processorIndex;
    String libName;
    int libVersion;
    bool isSource;
    bool isSink;
    int nodeId;
};

class ProcessorGraphHttpServer;


/**
  Owns all processors and constructs the signal chain.

  The GUI revolves around the ProcessorGraph, which enables the user to
  dynamically update the signal chain. This object creates and deletes
  all of the processors that handle data, and holds the rules for connecting
  them prior to data acquisition.

  The user is able to modify the ProcessGraph through the EditorViewport

  @see EditorViewport, GenericProcessor, GenericEditor, RecordNode,
       AudioNode, Configuration, MessageCenter
*/

class ProcessorGraph    : public AudioProcessorGraph
                        , public ChangeListener
{
public:

    /* IDs for default processors*/
    enum nodeIds
    {
        AUDIO_NODE_ID = 901,
        OUTPUT_NODE_ID = 902,
        MESSAGE_CENTER_ID = 904
    };

    /* Constructor*/
    ProcessorGraph();

    /* Destructor */
    ~ProcessorGraph();

    /* Creates a new processor.*/
    GenericProcessor* createProcessor(ProcessorDescription& description,
                         GenericProcessor* sourceNode = nullptr,
                         GenericProcessor* destNode = nullptr,
                         bool signalChainIsLoading=false);

    /* Determines which processor to create, based on the description provided*/
    std::unique_ptr<GenericProcessor> createProcessorFromDescription(ProcessorDescription& description);

    /* Starts the HTTP server for interacting with the GUI. */
    void enableHttpServer();

    /* Stops the HTTP server for interacting with the GUI. */
    void disableHttpServer();
    
    /* Checks whether an action has create the need for new 'root' processors (first in signal chain)*/
    bool checkForNewRootNodes(GenericProcessor* processor,
                              bool processorBeingAdded = true,
                              bool processorBeingMoved = false);
    
    /* Moves a processor to a new location in the signal chain. */
    void moveProcessor(GenericProcessor*, GenericProcessor* newSource = nullptr, GenericProcessor* newDest = nullptr,
                       bool moveDownstream = true);

    /* Remove a processor from the signal chain*/
    void removeProcessor(GenericProcessor* processor);

    /* Returns pointers to all of the processors in the signal chain*/
    Array<GenericProcessor*> getListOfProcessors();
    
    /* Finds a processor based on its ID*/
    GenericProcessor* getProcessorWithNodeId(int nodeId);
    
    /* Returns all of the 'root nodes' (first processors in signal chain)*/
    Array<GenericProcessor*> getRootNodes() {return rootNodes;}
    
    /* Returns a list of processor editors that are currently visible*/
    Array<GenericEditor*> getVisibleEditors(GenericProcessor* processor);

    /* Updates the settings of all processors downstream of the specified processor*/
    void updateSettings(GenericProcessor* processor, bool signalChainIsLoading = false);

    /* Updates the views (EditorViewport and GraphView) of all processors downstream of the specified processor*/
    void updateViews(GenericProcessor* processor, bool updateGraphViewer = false);

    /* Clears the signal chain.*/
    void clearSignalChain();

    /* Removes the specified processors.*/
    void deleteNodes(Array<GenericProcessor*> nodesToDelete);

    /* Calls startAcquisition() for all processors*/
    bool enableProcessors();

    /* Calls stopAcquisition() for all processors*/
    bool disableProcessors();

    /* Returns a list of all RecordNodes in the signal chain*/
    Array<RecordNode*> getRecordNodes();

    /* Returns a pointer to the AudioNode processor*/
    AudioNode* getAudioNode();

    /* Returns a pointer to the MessageCenter processor*/
    MessageCenter* getMessageCenter();
    
    /* Returns true if the signal chain has ast least one RecordNode*/
    bool hasRecordNode();

    /* Broadcasts a message to all processors during acquisition*/
    void broadcastMessage(String msg);

    /* Sends a configuration message to a particular processor, while acquisition is paused*/
    String sendConfigMessage(GenericProcessor* processor, String msg);

    /* Updates the connections in the signal chain*/
    void updateConnections();

    /* Returns true if there's an equivalent processor in the signal chain*/
    bool processorWithSameNameExists(const String& name);

    /* Respond to a change event*/
    void changeListenerCallback(ChangeBroadcaster* source);

    /** Loops through processors and restores parameters, if they're available. */
    void restoreParameters();
    
    /* Updates the buffer size used for the process() callbacks*/
    void updateBufferSize();

    /* Turns recording on (true) and off (false)*/
    void setRecordState(bool);

    /* Applies new colors to processors in the signal chain*/
    void refreshColors();

    /* Creates the nodes that exist in every signal chain (AudioNode, MessageCenter) */
    void createDefaultNodes();

    /* Makes a particular branch of the signal chain visible, without updating any settings */
    void viewSignalChain(int index);

	void setTimestampSource(int sourceIndex, int streamId);

	void getTimestampSources(Array<const GenericProcessor*>& validSources, int& selectedSource, int& selectedSubIdx) const;

	void getTimestampSources(int& selectedSource, int& selectedSubIdx) const;

    int64 getGlobalTimestamp() const;

    float getGlobalSampleRate() const;

    String getGlobalTimestampSource() const;

    void setTimestampWindow(TimestampSourceSelectionWindow* window);

	//uint32 getGlobalTimestampSourceFullId() const;

    static int getStreamIdxForChannel(Node& node, int channel);
    
private:

    /* Disconnect all processors*/
    void clearConnections();

    /* Connect a source processor and a destination processor*/
    void connectProcessors(GenericProcessor* source, 
        GenericProcessor* dest,
        bool connectContinuous, 
        bool connectEvents);

    /* Connect a processor to the AudioNode*/
    void connectAudioMonitorToAudioNode(GenericProcessor* source);

    /* Connect a processor to the MessageCenter*/
    void connectProcessorToMessageCenter(GenericProcessor* source);

	int64 m_startSoftTimestamp{ 0 };
	const GenericProcessor* m_timestampSource{ nullptr };
	int m_timestampSourceStreamId;
	Array<const GenericProcessor*> m_validTimestampSources;
	WeakReference<TimestampSourceSelectionWindow> m_timestampWindow;
    
    Array<GenericProcessor*> rootNodes;

    int currentNodeId;

    bool isLoadingSignalChain;

    std::unique_ptr<ProcessorGraphHttpServer> http_server_thread;

};



#endif  // __PROCESSORGRAPH_H_124F8B50__
