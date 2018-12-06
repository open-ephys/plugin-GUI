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
    StringTS(String S, int64 ts_software = CoreServices::getGlobalTimestamp());
    StringTS(MidiMessage& event);

	std::vector<String> splitString(char sep) const;

	String str;
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

    std::vector<String> splitString (String S, char sep);

    void initSimulation();
    void simulateDesignAndTrials ();
    void simulateSingleTrial();
    void simulateStartRecord();
    void simulateStopRecord();
    void run();
    void opensocket();
    void closesocket(bool shutdown = false);

    void setNewListeningPort (uint16 port);

    uint16 urlport;

private:
    void createZmqContext();

    void postTimestamppedStringToMidiBuffer(StringTS s);
    
    String handleSpecialMessages(const String& s);

    //* Split network message into name/value pairs (name1=val1 name2=val2 etc) */
    StringPairArray parseNetworkMessage(StringRef msg);
    
    // Set the urlport and reflect it on the editor
    void updatePort(uint16 port);

    // RAII wrapper for responder socket
    class Responder
    {
    public:
        // creates socket from given context and tries to bind to port, then backupPort (if nonzero).
        // if port is 0, chooses an available ephemeral port.
        Responder(void* context, uint16 port, uint16 backupPort);
        ~Responder();

        bool initialize();

        // returns the latest errno value and resets it to 0.
        int getErr();
        
        // returns the port if the socket was successfully bound to one.
        // if not, or if the socket is invalid, returns 0.
        uint16 getBoundPort() const;

        // receives message into buf (blocking call).
        // returns the number of bytes actually received, or -1 if there is an error.
        int receive(void* buf);

        // sends a message. returns the same as zmq_send.
        int send(StringRef response);

    private:
        void* socket;
        uint16 boundPort; // 0 indicates not bound
        int lastErrno;
        bool initialized;
    };
    
    void* zmqcontext;
    uint16 lastGoodPort;

    float threshold;
    float bufferZone;

    bool state;
    bool firstTime;

    Time timer;

    std::queue<StringTS> networkMessagesQueue;
    std::queue<StringTS> simulation;

    CriticalSection queueLock;
    
    int64 simulationStartTime;

	const EventChannel* messageChannel{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkEvents);
};

#endif  // __NETWORKEVENT_H_91811541__
