/*
  ==============================================================================

    EventBroadcaster.cpp
    Created: 22 May 2015 3:31:50pm
    Author:  Christopher Stawarz

  ==============================================================================
*/

#include "EventBroadcaster.h"
#include "EventBroadcasterEditor.h"

std::shared_ptr<void> EventBroadcaster::getZMQContext() {
    // Note: C++11 guarantees that initialization of static local variables occurs exactly once, even
    // if multiple threads attempt to initialize the same static local variable concurrently.
#ifdef ZEROMQ
    static const std::shared_ptr<void> ctx(zmq_ctx_new(), zmq_ctx_destroy);
#else
    static const std::shared_ptr<void> ctx;
#endif
    return ctx;
}


void EventBroadcaster::closeZMQSocket(void* socket)
{
#ifdef ZEROMQ
    zmq_close(socket);
#endif
}


EventBroadcaster::EventBroadcaster()
    : GenericProcessor  ("Event Broadcaster")
    , zmqContext        (getZMQContext())
    , zmqSocket         (nullptr, &closeZMQSocket)
    , listeningPort     (0)
    , currentSampleRate (0)
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    setListeningPort(5557);
}


AudioProcessorEditor* EventBroadcaster::createEditor()
{
    editor = new EventBroadcasterEditor(this, true);
    return editor;
}


int EventBroadcaster::getListeningPort() const
{
    return listeningPort;
}


void EventBroadcaster::setListeningPort(int port, bool forceRestart)
{
    if ((listeningPort != port) || forceRestart)
    {
#ifdef ZEROMQ
        zmqSocket.reset(zmq_socket(zmqContext.get(), ZMQ_PUB));
        if (!zmqSocket)
        {
            std::cout << "Failed to create socket: " << zmq_strerror(zmq_errno()) << std::endl;
            return;
        }

        String url = String("tcp://*:") + String(port);
        if (0 != zmq_bind(zmqSocket.get(), url.toRawUTF8()))
        {
            std::cout << "Failed to open socket: " << zmq_strerror(zmq_errno()) << std::endl;
            return;
        }
#endif

        listeningPort = port;
    }
}


void EventBroadcaster::process(AudioSampleBuffer& continuousBuffer, MidiBuffer& eventBuffer)
{
    currentSampleRate = getSampleRate();
    checkForEvents(eventBuffer);
}


void EventBroadcaster::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{
    const uint8_t* buffer = event.getRawData();
    uint8_t type = buffer[0];
    int64_t timestamp;
    
    switch (type) {
        case TTL:
        case MESSAGE:
        case BINARY_MSG: {
            uint8_t nodeID = buffer[1];
            timestamp = timestamps.at(nodeID) + samplePosition;
            break;
        }
            
        case SPIKE:
            std::copy_n(buffer + 1, sizeof(timestamp), reinterpret_cast<uint8_t *>(&timestamp));
            break;
            
        default:
            // Don't broadcast other event types
            return;
    }
    
    double timestampSeconds = double(timestamp) / currentSampleRate;

#ifdef ZEROMQ
    if (-1 == zmq_send(zmqSocket.get(), &type, sizeof(type), ZMQ_SNDMORE) ||
        -1 == zmq_send(zmqSocket.get(), &timestampSeconds, sizeof(timestampSeconds), ZMQ_SNDMORE) ||
        -1 == zmq_send(zmqSocket.get(), buffer + 1, event.getRawDataSize() - 1, 0) /* Omit event type */)
    {
        std::cout << "Failed to send message: " << zmq_strerror(zmq_errno()) << std::endl;
    }
#endif
}


void EventBroadcaster::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("EVENTBROADCASTER");
    mainNode->setAttribute("port", listeningPort);
}


void EventBroadcaster::loadCustomParametersFromXml()
{
    if (parametersAsXml)
    {
        forEachXmlChildElement(*parametersAsXml, mainNode)
        {
            if (mainNode->hasTagName("EVENTBROADCASTER"))
            {
                setListeningPort(mainNode->getIntAttribute("port"));
            }
        }
    }
}
