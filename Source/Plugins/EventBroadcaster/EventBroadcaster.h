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

#include <memory>


class EventBroadcaster : public GenericProcessor
{
public:
    EventBroadcaster();

    AudioProcessorEditor* createEditor() override;

    int getListeningPort() const;
    // returns 0 on success, else the errno value for the error that occurred.
    int setListeningPort (int port, bool forceRestart = false);

    void process (AudioSampleBuffer& continuousBuffer) override;
    void handleEvent (const EventChannel* channelInfo, const MidiMessage& event, int samplePosition = 0) override;
	void handleSpike(const SpikeChannel* channelInfo, const MidiMessage& event, int samplePosition = 0) override;

    void saveCustomParametersToXml (XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;


private:
	void sendEvent(const MidiMessage& event, float eventSampleRate) const;
    static std::shared_ptr<void> getZMQContext();
    int unbindZMQSocket();
    int rebindZMQSocket();
    static void closeZMQSocket (void* socket);
    static String getEndpoint(int port);
    // called from getListeningPort() depending on success/failure of ZMQ operations
    void reportActualListeningPort(int port);

    // encapuslate closing sockets when their pointers go out of scope
    struct zmqSocketPtr : public std::unique_ptr<void, decltype(&closeZMQSocket)>
    {
        zmqSocketPtr(void* ptr) 
            : std::unique_ptr<void, decltype(&closeZMQSocket)>(ptr, &closeZMQSocket)
        {}
    };

    const std::shared_ptr<void> zmqContext;
    zmqSocketPtr zmqSocket;
    int listeningPort;

};


#endif  // EVENTBROADCASTER_H_INCLUDED
