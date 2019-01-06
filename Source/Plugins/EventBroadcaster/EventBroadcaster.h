/*
  ==============================================================================

    EventBroadcaster.h
    Created: 22 May 2015 3:31:50pm
    Author:  Christopher Stawarz

  ==============================================================================
*/

#ifndef EVENTBROADCASTER_H_INCLUDED
#define EVENTBROADCASTER_H_INCLUDED

#include <ProcessorHeaders.h>

#ifdef ZEROMQ
    #ifdef WIN32
        #include <zmq.h>
        #include <zmq_utils.h>
    #else
        #include <zmq.h>
    #endif
#endif

class EventBroadcaster : public GenericProcessor
{
public:
    EventBroadcaster();

    AudioProcessorEditor* createEditor() override;

    int getListeningPort() const;
    // returns 0 on success, else the errno value for the error that occurred.
    int setListeningPort(int port, bool forceRestart = false);

    void process (AudioSampleBuffer& continuousBuffer) override;
    void handleEvent (const EventChannel* channelInfo, const MidiMessage& event, int samplePosition = 0) override;
	void handleSpike(const SpikeChannel* channelInfo, const MidiMessage& event, int samplePosition = 0) override;

    void saveCustomParametersToXml (XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;


private:
    class ZMQContext
    {
    public:
        ZMQContext();
        ~ZMQContext();
        void* createZMQSocket();
    private:
        void* context;
    };

    class ZMQSocket
    {
    public:
        ZMQSocket();
        ~ZMQSocket();

        bool isValid() const;
        int getBoundPort() const;

        int send(const void* buf, size_t len, int flags);
        int bind(int port);
        int unbind();
    private:
        int boundPort;
        void* socket;

        // see here for why the context can't just be static:
        // https://github.com/zeromq/libzmq/issues/1708
        SharedResourcePointer<ZMQContext> context;
    };

	void sendEvent(const MidiMessage& event, float eventSampleRate) const;

    static String getEndpoint(int port);
    
    ScopedPointer<ZMQSocket> zmqSocket;
};


#endif  // EVENTBROADCASTER_H_INCLUDED
