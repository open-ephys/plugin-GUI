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

#ifndef PSTHNODE_H_
#define PSTHNODE_H_

#include "../../JuceLibraryCode/JuceHeader.h"
//#include "../Editors/SpikeDisplayEditor.h"
#include "../Editors/VisualizerEditor.h"
#include "../NetworkEvents/NetworkEvents.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../Visualization/SpikeObject.h"
#include "TrialCircularBuffer.h"
//#include "ISCAN.h"
#include <queue>
#include <vector>

class DataViewport;
class SpikePlot;
class TrialCircularBuffer;

class PeriStimulusTimeHistogramNode :  public GenericProcessor
{
public:

    PeriStimulusTimeHistogramNode();
    ~PeriStimulusTimeHistogramNode();

    AudioProcessorEditor* createEditor();
	
	void toggleConditionVisibility(int cond);

    bool isSink()
    {
        return true;
    }

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    void syncInternalDataStructuresWithSpikeSorter();

	void allocateTrialCircularBuffer();
    void handleEvent(int, MidiMessage&, int);

    void updateSettings();

    bool enable();
    bool disable();

    String getNameForElectrode(int i);
    int getNumberOfChannelsForElectrode(int i);
    int getNumElectrodes();

    void addSpikePlotForElectrode(SpikePlot* sp, int i);
    void removeSpikePlots();
	void reallocate(int numChannels);
	void stopRecording();
	void startRecording();
	void saveCustomParametersToXml(XmlElement* parentElement);
	void loadCustomParametersFromXml();
	void modifyTimeRange(double preSec_, double postSec_);

	ScopedPointer<TrialCircularBuffer> trialCircularBuffer;
	bool saveTTLs, saveNetworkEvents,saveEyeTracking ;
	int spikeSavingMode;
	bool saveNetworkEventsWhenNotRecording;

    void setHardwareTriggerAlignmentChannel(int chan);
	
	void handleNetworkMessage(StringTS s);
private:

	bool isRecording;
    int displayBufferSize;
    bool redrawRequested;
	int syncCounter;
	int64 hardware_timestamp, software_timestamp;

	std::queue<StringTS> networkEventsHistory;
    RecordNode* recordNode;
//    uint16 recordingNumber;
    CriticalSection diskWriteLock;
	Array<Channel*> electrodeChannels;

    Time timer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramNode);

};


#endif // PSTHNODE_H_
