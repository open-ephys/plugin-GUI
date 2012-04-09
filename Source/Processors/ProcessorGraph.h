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

#ifndef __PROCESSORGRAPH_H_124F8B50__
#define __PROCESSORGRAPH_H_124F8B50__

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../AccessClass.h"

/**
  
  Owns all processors and constructs the signal chain.

  The GUI revolves around the ProcessorGraph, which enables the user to 
  dynamically update the signal chain. This object creates and deletes
  all of the processors that handle data, and holds the rules for connecting
  them prior to data acquisition.

  @see FilterViewport, GenericProcessor, GenericEditor, RecordNode,
       AudioNode, Configuration, MessageCenter

*/


class GenericProcessor;
class RecordNode;
class AudioNode;
class SignalChainTabButton;

class ProcessorGraph : public AudioProcessorGraph,
					   public AccessClass
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
	AudioNode* getAudioNode();

	void updateConnections(Array<SignalChainTabButton*, CriticalSection>);

	bool processorWithSameNameExists(const String& name);

	void saveState();
	void loadState();

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
