/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../AccessClass.h"

class GenericProcessor;
class RecordNode;
class AudioNode;
class SignalChainTabButton;

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

class ProcessorGraph : public AudioProcessorGraph,
    public AccessClass,
    public ChangeListener
{
public:
    ProcessorGraph();
    ~ProcessorGraph();

    void* createNewProcessor(String& description);
    GenericProcessor* createProcessorFromDescription(String& description);

    void removeProcessor(GenericProcessor* processor);

    void clearSignalChain();

    bool enableProcessors();
    bool disableProcessors();

    RecordNode* getRecordNode();
    AudioNode* getAudioNode();

    void updateConnections(Array<SignalChainTabButton*, CriticalSection>);

    bool processorWithSameNameExists(const String& name);

    void changeListenerCallback(ChangeBroadcaster* source);

    /** Loops through processors and restores parameters, if they're available. */
    void restoreParameters();

    void updatePointers();
    
    void setRecordState(bool);

    Array<GenericProcessor*> getListOfProcessors();

private:

    int currentNodeId;

    enum nodeIds
    {
        RECORD_NODE_ID = 900,
        AUDIO_NODE_ID = 901,
        OUTPUT_NODE_ID = 902,
        RESAMPLING_NODE_ID = 903
    };

    void createDefaultNodes();

    void clearConnections();

};



#endif  // __PROCESSORGRAPH_H_124F8B50__
