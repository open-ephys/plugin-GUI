/*
  ==============================================================================

    EventBroadcaster.cpp
    Created: 22 May 2015 3:31:50pm
    Author:  Christopher Stawarz

  ==============================================================================
*/

#include "EventBroadcaster.h"
#include "EventBroadcasterEditor.h"

EventBroadcaster::ZMQContext* EventBroadcaster::sharedContext = nullptr;
CriticalSection EventBroadcaster::sharedContextLock{};

EventBroadcaster::ZMQContext::ZMQContext(const ScopedLock& lock)
#ifdef ZEROMQ
    : context(zmq_ctx_new())
#endif
{
    sharedContext = this;
}

// ZMQContext is a ReferenceCountedObject with a pointer in each instance's 
// socket pointer, so this only happens when the last instance is destroyed.
EventBroadcaster::ZMQContext::~ZMQContext()
{
    ScopedLock lock(sharedContextLock);
    sharedContext = nullptr;
#ifdef ZEROMQ
    zmq_ctx_destroy(context);
#endif
}

void* EventBroadcaster::ZMQContext::createZMQSocket()
{
#ifdef ZEROMQ
    jassert(context != nullptr);
    return zmq_socket(context, ZMQ_PUB);
#endif
}

EventBroadcaster::ZMQSocket::ZMQSocket()
    : socket(nullptr)
{
    ScopedLock lock(sharedContextLock);
    if (sharedContext == nullptr)
    {
        // first one, create the context
        context = new ZMQContext(lock);
    }
    else
    {
        // use already-created context
        context = sharedContext;
    }

#ifdef ZEROMQ
    socket = context->createZMQSocket();
#endif
}

EventBroadcaster::ZMQSocket::~ZMQSocket()
{
#ifdef ZEROMQ
    zmq_close(socket);
#endif
}

bool EventBroadcaster::ZMQSocket::isValid() const
{
    return socket != nullptr;
}

int EventBroadcaster::ZMQSocket::send(const void* buf, size_t len, int flags)
{
#ifdef ZEROMQ
    return zmq_send(socket, buf, len, flags);
#endif
    return 0;
}

int EventBroadcaster::ZMQSocket::bind(int port)
{
#ifdef ZEROMQ
    if (isValid() && port != 0)
    {
        return zmq_bind(socket, getEndpoint(port).toRawUTF8());
    }
#endif
    return 0;
}

int EventBroadcaster::ZMQSocket::unbind(int port)
{
#ifdef ZEROMQ
    if (isValid() && port != 0)
    {
        return zmq_unbind(socket, getEndpoint(port).toRawUTF8());
    }
#endif
    return 0;
}


String EventBroadcaster::getEndpoint(int port)
{
    return String("tcp://*:") + String(port);
}

void EventBroadcaster::reportActualListeningPort(int port)
{
    listeningPort = port;
    auto editor = static_cast<EventBroadcasterEditor*>(getEditor());
    if (editor)
    {
        editor->setDisplayedPort(port);
    }
}

EventBroadcaster::EventBroadcaster()
    : GenericProcessor  ("Event Broadcaster")
    , listeningPort     (0)
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    int portToTry = 5557;
    while (setListeningPort(portToTry) == EADDRINUSE)
    {
        // try the next port, looking for one not in use
        portToTry++;
    }
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


int EventBroadcaster::setListeningPort(int port, bool forceRestart)
{
    if ((listeningPort != port) || forceRestart)
    {
#ifdef ZEROMQ
        // unbind current socket (if any) to free up port
        if (zmqSocket != nullptr)
        {
            zmqSocket->unbind(listeningPort);
        }

        ScopedPointer<ZMQSocket> newSocket = new ZMQSocket();
        auto editor = static_cast<EventBroadcasterEditor*>(getEditor());
        int status = 0;

        if (!newSocket->isValid())
        {
            status = zmq_errno();
            std::cout << "Failed to create socket: " << zmq_strerror(status) << std::endl;
        }
        else
        {
            if (0 != newSocket->bind(port))
            {
                status = zmq_errno();
                std::cout << "Failed to create socket: " << zmq_strerror(status) << std::endl;
            }
            else
            {
                // success
                zmqSocket = newSocket;
                reportActualListeningPort(port);
                return status;
            }
        }

        // failure, try to rebind current socket to previous port
        if (zmqSocket != nullptr && 0 == zmqSocket->bind(listeningPort))
        {
            reportActualListeningPort(listeningPort);
        }
        else
        {
            reportActualListeningPort(0);
        }
        return status;
#else
        reportActualListeningPort(port);
        return 0;
#endif
    }
    return -1;
}


void EventBroadcaster::process(AudioSampleBuffer& continuousBuffer)
{
    checkForEvents(true);
}


//IMPORTANT: The structure of the event buffers has changed drastically, so we need to find a better way of doing this
void EventBroadcaster::sendEvent(const MidiMessage& event, float eventSampleRate) const
{
#ifdef ZEROMQ
	double timestampSeconds = double(Event::getTimestamp(event)) / eventSampleRate;
	uint16 type = Event::getBaseType(event);

    if (!zmqSocket)
    {
        std::cout << "Failed to send message: no socket" << std::endl;
    }
	else if (-1 == zmqSocket->send(&type, sizeof(type), ZMQ_SNDMORE) ||
		     -1 == zmqSocket->send(&timestampSeconds, sizeof(timestampSeconds), ZMQ_SNDMORE) ||
		     -1 == zmqSocket->send(event.getRawData(), event.getRawDataSize(), 0))
	{
		std::cout << "Failed to send message: " << zmq_strerror(zmq_errno()) << std::endl;
	}
#endif
}

void EventBroadcaster::handleEvent(const EventChannel* channelInfo, const MidiMessage& event, int samplePosition)
{
	sendEvent(event, channelInfo->getSampleRate());
}

void EventBroadcaster::handleSpike(const SpikeChannel* channelInfo, const MidiMessage& event, int samplePosition)
{
	sendEvent(event, channelInfo->getSampleRate());
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
