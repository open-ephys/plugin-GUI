/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2015 Open Ephys

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

#ifndef __NETWORKEVENT_H_91811541__
#define __NETWORKEVENT_H_91811541__
//#define ZEROMQ

#ifdef ZEROMQ

#ifdef WIN32
//#pragma comment( lib, "../../Resources/windows-libs/ZeroMQ/lib_x64/libzmq-v120-mt-4_0_4.lib" )
#include <zmq.h>
#include <zmq_utils.h>
#else
#include <zmq.h>
#endif

#endif

#include <ProcessorHeaders.h>

#include <list>
#include <queue>

/**

 Sends incoming TCP/IP messages from 0MQ to the events buffer

  @see GenericProcessor

*/

class StringTS
{
public:
    StringTS();
    std::vector<String> splitString(char sep);
    StringTS(MidiMessage& event);
    String getString();
    StringTS(String S);
    StringTS(String S, int64 ts_software);
    StringTS(const StringTS& s);
    StringTS(unsigned char* buf, int _len, int64 ts_software);
    StringTS& operator=(const StringTS& rhs);
    ~StringTS();

    juce::uint8* str;
    int len;
    juce::int64 timestamp;
};

class NetworkEvents : public GenericProcessor,  public Thread
{
public:
    NetworkEvents();
    ~NetworkEvents();
    AudioProcessorEditor* createEditor();
    int64 getExtrapolatedHardwareTimestamp(int64 softwareTS);
    void initSimulation();
    void simulateDesignAndTrials(juce::MidiBuffer& events);
    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void setParameter(int parameterIndex, float newValue);
    String handleSpecialMessages(StringTS msg);
    std::vector<String> splitString(String S, char sep);

    void simulateSingleTrial();
    bool isSource();

    void simulateStartRecord();
    void simulateStopRecord();
    bool closesocket();
    void run();
    void opensocket();

    void updateSettings();

    bool isReady();
    float getDefaultSampleRate();
    int getDefaultNumOutputs();
    float getDefaultBitVolts();
    void enabledState(bool t);

    int getNumEventChannels();

    void postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events);
    void setNewListeningPort(int port);

    void saveCustomParametersToXml(XmlElement* parentElement);
    void loadCustomParametersFromXml();

    int urlport;
    String socketStatus;
    bool threadRunning ;
private:
    void handleEvent(int eventType, MidiMessage& event, int samplePos);
    void createZmqContext();

	//* Split network message into name/value pairs (name1=val1 name2=val2 etc) */
	StringPairArray parseNetworkMessage(String msg);

    StringTS createStringTS(String S, int64 t);

    static void* zmqcontext;
    void* responder;
    float threshold;
    float bufferZone;
    bool state;
    bool shutdown;
    Time timer;
    std::queue<StringTS> networkMessagesQueue;

    CriticalSection lock;
    std::queue<StringTS> simulation;
    int64 simulationStartTime;
    bool firstTime ;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetworkEvents);

};

#endif  // __NETWORKEVENT_H_91811541__
