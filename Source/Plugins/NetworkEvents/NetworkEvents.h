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

#include <vector>
#include <list>
#include <queue>


/**
 Sends incoming TCP/IP messages from 0MQ to the events buffer

  @see GenericProcessor
*/
class NetworkEvents : public GenericProcessor
                    , public Thread
                    , public Value::Listener
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

    // passing 0 corresponds to wildcard ("*") and picks any available port
    // returns true on success, false on failure
    bool setNewListeningPort (uint16 port);

    // gets a string for the editor's port input to reflect current urlport
    String getPortString() const;

    void restartConnection();

    // to update the port string from the thread
    void valueChanged(Value& value) override;

private:
    // combines a string and a timestamp
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

    class ZMQContext : public ReferenceCountedObject
    {
    public:
        ZMQContext(const ScopedLock& lock);
        ~ZMQContext() override;
        void* createSocket();

        typedef ReferenceCountedObjectPtr<ZMQContext> Ptr;

    private:
        void* context;
    };

    // RAII wrapper for REP socket
    class Responder
    {
    public:
        // tries to create a responder and bind to given port; returns nullptr on failure.
        // caller must own and destroy the returned responder if it succeeds.
        static Responder* makeResponder(uint16 port);
        ~Responder();

        // returns the latest errno value
        int getErr() const;

        // output last error on stdout and status bar, including the passed message
        void reportErr(const String& message) const;

        // returns the port if the socket was successfully bound to one.
        // if not, or if the socket is invalid, returns 0.
        uint16 getBoundPort() const;

        // receives message into buf (blocking call).
        // returns the number of bytes actually received, or -1 if there is an error.
        int receive(void* buf);

        // sends a message. returns the same as zmq_send.
        int send(StringRef response);

    private:
        // creates socket from given context and tries to bind to port.
        // if port is 0, chooses an available ephemeral port.
        Responder(uint16 port);

        ZMQContext::Ptr context;
        void* socket;
        uint16 boundPort; // 0 indicates not bound
        int lastErrno;

        static const int RECV_TIMEOUT_MS = 100;
    };

    void postTimestamppedStringToMidiBuffer(StringTS s);
    
    String handleSpecialMessages(const String& s);

    //* Split network message into name/value pairs (name1=val1 name2=val2 etc) */
    StringPairArray parseNetworkMessage(StringRef msg);

    // updates urlport and the portString Value (controlling the port input on the editor)
    // 0 indicates disconnected. should only be called from the thread!
    void updatePort(uint16 port);

    // get an endpoint url for the given port (using 0 to represent *)
    static String getEndpoint(uint16 port);

    Value portString; // underlying value of the editor's port input
     
    // share a "dumb" pointer that doesn't take part in reference counting.
    // want the context to be terminated by the time the static members are
    // destroyed (see: https://github.com/zeromq/libzmq/issues/1708)
    static ZMQContext* sharedContext;
    static CriticalSection sharedContextLock;

    // To switch ports, a new socket is created and (if successful) assigned to this pointer,
    // and then the thread will switch to using this socket at the next opportunity.
    ScopedPointer<Responder> nextResponder;
    CriticalSection nextResponderLock;

    uint16 urlport;  // 0 indicates not connected

    Atomic<int> restart;

    float threshold;
    float bufferZone;

    bool state;
    bool firstTime;

    std::queue<StringTS> networkMessagesQueue;
    std::queue<StringTS> simulation;

    CriticalSection queueLock;
    
    int64 simulationStartTime;

	const EventChannel* messageChannel{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkEvents);
};

#endif  // __NETWORKEVENT_H_91811541__
