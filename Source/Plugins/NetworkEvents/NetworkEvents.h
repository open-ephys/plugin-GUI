/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

class StringTS
{
public:
	StringTS();
	StringTS(MidiMessage& event);
	StringTS(String S);
	StringTS(String S, int64 ts_software);
	StringTS(const StringTS& s);
	StringTS(unsigned char* buf, int _len, int64 ts_software);
	~StringTS();

	std::vector<String> splitString(char sep);
	String getString();

	StringTS& operator= (const StringTS& rhs);

	juce::uint8* str;
	int len;
	juce::int64 timestamp;
};


/**
 Sends incoming TCP/IP messages from 0MQ to the events buffer

  @see GenericProcessor
*/
class NetworkEvents : public GenericProcessor
                    , public Thread
{
public:
    NetworkEvents();
    ~NetworkEvents();

    // GenericProcessor methods
    // =========================================================================
    AudioProcessorEditor* createEditor() override;

    void process (AudioSampleBuffer& buffer) override;

    void setParameter (int parameterIndex, float newValue) override;

    void createEventChannels() override;

    void setEnabledState (bool newState) override;

    void saveCustomParametersToXml (XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;

    bool isReady() override;

    float getDefaultSampleRate() const override;
    float getDefaultBitVolts()   const override;

    // =========================================================================

    int getDefaultNumOutputs() const;

    //int64 getExtrapolatedHardwareTimestamp (int64 softwareTS) const;

    String handleSpecialMessages    (StringTS msg);
    std::vector<String> splitString (String S, char sep);

    void initSimulation();
    void simulateDesignAndTrials ();
    void simulateSingleTrial();
    void simulateStartRecord();
    void simulateStopRecord();
    void run();
    void opensocket();
    bool closesocket();

    void postTimestamppedStringToMidiBuffer (StringTS s);
    void setNewListeningPort (int port);

    int urlport;
    String socketStatus;
    std::atomic<bool> threadRunning;


private:
    void createZmqContext();

    //* Split network message into name/value pairs (name1=val1 name2=val2 etc) */
    StringPairArray parseNetworkMessage (String msg);

    StringTS createStringTS (String S, int64 t);

    static void* zmqcontext;
    void* responder;

    float threshold;
    float bufferZone;

    bool state;
    bool shutdown;
    bool firstTime;

    Time timer;

    std::queue<StringTS> networkMessagesQueue;
    std::queue<StringTS> simulation;

    CriticalSection lock;
    int64 simulationStartTime;

	const EventChannel* messageChannel{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkEvents);
};

#endif  // __NETWORKEVENT_H_91811541__
