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
#include <atomic>

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

    void createEventChannels() override;

    void setEnabledState (bool newState) override;

    void saveCustomParametersToXml (XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;

    bool isReady() override;

    float getDefaultSampleRate() const override;
    float getDefaultBitVolts()   const override;

    // =========================================================================

    void run() override;

    // passing 0 corresponds to wildcard ("*") and picks any available port
    void setNewListeningPort (uint16 port);

    // gets a string for the editor's port input to reflect current urlport
    String getCurrPortString() const;

    void restartConnection();

private:
    struct StringTS
    {
        String str;
        int64 timestamp;
    };

    class ZMQContext
    {
    public:
        ZMQContext();
        ~ZMQContext();
        void* createSocket();
    private:
        void* context;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZMQContext);
    };

    // RAII wrapper for REP socket
    class Responder
    {
    public:
        // creates socket from given context and tries to bind to port.
        // if port is 0, chooses an available ephemeral port.
        Responder(uint16 port);
        ~Responder();

        // returns the latest errno value
        int getErr() const;

        // output last error on stdout and status bar, including the passed message
        void reportErr(const String& message) const;

        bool isValid() const;

        // returns the port if the socket was successfully bound to one, else 0
        // if not, or if the socket is invalid, returns 0.
        uint16 getBoundPort() const;

        // receives message into buf (blocking call).
        // returns the number of bytes actually received, or -1 if there is an error.
        int receive(void* buf);

        // sends a message. returns the same as zmq_send.
        int send(StringRef response);

    private:
        SharedResourcePointer<ZMQContext> context;
        void* socket;
        bool valid;
        uint16 boundPort;
        int lastErrno;

        static const int RECV_TIMEOUT_MS;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Responder);
    };

    void postTimestamppedStringToMidiBuffer(const StringTS& s);
    
    String handleSpecialMessages(const String& s);

    //* Split network message into name/value pairs (name1=val1 name2=val2 etc) */
    StringPairArray parseNetworkMessage(StringRef msg);

    // updates urlport and the port input on the editor (< 0 indicates not connected)
    void updatePortString(uint16 port);

    // get an endpoint url for the given port (using 0 to represent *)
    static String getEndpoint(uint16 port);

    // get a representation of the given port for use on the editor
    static String getPortString(uint16 port);

    std::atomic<bool> makeNewSocket;   // port change or restart needed (depending on requestedPort)
    std::atomic<uint16> requestedPort; // never set by the thread; 0 means any free port
    std::atomic<uint16> boundPort;     // only set by the thread; 0 means no connection

    std::queue<StringTS> networkMessagesQueue;
    CriticalSection queueLock;
    
	const EventChannel* messageChannel{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkEvents);
};

#endif  // __NETWORKEVENT_H_91811541__
