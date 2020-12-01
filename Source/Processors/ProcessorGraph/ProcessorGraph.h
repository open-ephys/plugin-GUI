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
    ProcessorGraph();
    ~ProcessorGraph();

    GenericProcessor* createProcessor(ProcessorDescription& description,
                         GenericProcessor* sourceNode = nullptr,
                         GenericProcessor* destNode = nullptr,
                         bool signalChainIsLoading=false);
    GenericProcessor* createProcessorFromDescription(ProcessorDescription& description);
    
    bool checkForNewRootNodes(GenericProcessor* processor,
                              bool processorBeingAdded = true,
                              bool processorBeingMoved = false);
    
    void moveProcessor(GenericProcessor*, GenericProcessor* newSource = nullptr, GenericProcessor* newDest = nullptr,
                       bool moveDownstream = true);

    void removeProcessor(GenericProcessor* processor);
    Array<GenericProcessor*> getListOfProcessors();
    
    
    Array<GenericProcessor*> getRootNodes() {return rootNodes;}
    
    Array<GenericEditor*> getVisibleEditors(GenericProcessor* processor);

    
    void updateSettings(GenericProcessor* processor, bool signalChainIsLoading = false);
    void updateViews(GenericProcessor* processor);
    void clearSignalChain();
    void deleteNodes(Array<GenericProcessor*> nodesToDelete);

    bool enableProcessors();
    bool disableProcessors();

    Array<RecordNode*> getRecordNodes();
    AudioNode* getAudioNode();
    MessageCenter* getMessageCenter();
    
    bool hasRecordNode();

    void updateConnections();

    bool processorWithSameNameExists(const String& name);

    void changeListenerCallback(ChangeBroadcaster* source);

    /** Loops through processors and restores parameters, if they're available. */
    void restoreParameters();
    
    //void loadParametersFromXml(GenericProcessor*);

    void updatePointers();

    void setRecordState(bool);

    void refreshColors();

    void createDefaultNodes();

	void setTimestampSource(int sourceIndex, int subIdx);

	void getTimestampSources(Array<const GenericProcessor*>& validSources, int& selectedSource, int& selectedSubIdx) const;

	void getTimestampSources(int& selectedSource, int& selectedSubIdx) const;

	int64 getGlobalTimestamp(bool softwareOnly) const;

	float getGlobalSampleRate(bool softwareOnly) const;

	uint32 getGlobalTimestampSourceFullId() const;

	void setTimestampWindow(TimestampSourceSelectionWindow* window);
    
    void viewSignalChain(int index);

private:
    int currentNodeId;
    
    bool isLoadingSignalChain;

    enum nodeIds
    {
        RECORD_NODE_ID = 900,
        AUDIO_NODE_ID = 901,
        OUTPUT_NODE_ID = 902,
        MESSAGE_CENTER_ID = 904
    };

    void clearConnections();

    void connectProcessors(GenericProcessor* source, GenericProcessor* dest,
        bool connectContinuous, bool connectEvents);
    void connectProcessorToAudioNode(GenericProcessor* source);
    void connectProcessorToMessageCenter(GenericProcessor* source);

	int64 m_startSoftTimestamp{ 0 };
	const GenericProcessor* m_timestampSource{ nullptr };
	int m_timestampSourceSubIdx;
	Array<const GenericProcessor*> m_validTimestampSources;
	WeakReference<TimestampSourceSelectionWindow> m_timestampWindow;
    
    Array<GenericProcessor*> rootNodes;

};



#endif  // __PROCESSORGRAPH_H_124F8B50__
