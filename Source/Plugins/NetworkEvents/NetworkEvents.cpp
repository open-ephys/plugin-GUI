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

#include <stdio.h>
#include "NetworkEvents.h"
#include "NetworkEventsEditor.h"


const int MAX_MESSAGE_LENGTH = 64000;


#ifdef WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

NetworkEvents::ZMQContext* NetworkEvents::sharedContext = nullptr;
CriticalSection NetworkEvents::sharedContextLock{};

NetworkEvents::NetworkEvents()
    : GenericProcessor  ("Network Events")
    , Thread            ("NetworkThread")
    , makeNewSocket     (true)
    , requestedPort     (5556)
    , boundPort         (0)
{
    setProcessorType (PROCESSOR_TYPE_SOURCE);

    startThread();

    sendSampleCount = false; // disable updating the continuous buffer sample counts,
    // since this processor only sends events
}


void NetworkEvents::setNewListeningPort(uint16 port)
{
    requestedPort = port;
    makeNewSocket = true;
}


NetworkEvents::~NetworkEvents()
{
    if (!stopThread(1000))
    {
        jassertfalse; // shouldn't block for more than 100 ms, something's wrong
        std::cerr << "Network thread timeout. Forcing thread termination, system could be left in an unstable state" << std::endl;
    }
}


String NetworkEvents::getCurrPortString() const
{
    return getPortString(boundPort);
}


void NetworkEvents::restartConnection()
{
    makeNewSocket = true;
}


void NetworkEvents::createEventChannels()
{
	EventChannel* chan = new EventChannel(EventChannel::TEXT, 1, MAX_MESSAGE_LENGTH, CoreServices::getGlobalSampleRate(), this);
	chan->setName("Network messages");
	chan->setDescription("Messages received through the network events module");
	chan->setIdentifier("external.network.rawData");
	chan->addEventMetaData(new MetaDataDescriptor(MetaDataDescriptor::INT64, 1, "Software timestamp",
		"OS high resolution timer count when the event was received", "timestamp.software"));
	eventChannelArray.add(chan);
	messageChannel = chan;
}


AudioProcessorEditor* NetworkEvents::createEditor ()
{
    editor = new NetworkEventsEditor (this, true);

    return editor;
}


void NetworkEvents::postTimestamppedStringToMidiBuffer (const StringTS& s)
{
	MetaDataValueArray md;
	md.add(new MetaDataValue(MetaDataDescriptor::INT64, 1, &s.timestamp));
	TextEventPtr event = TextEvent::createTextEvent(messageChannel, CoreServices::getGlobalTimestamp(), s.str, md);
	addEvent(messageChannel, event, 0);
}


String NetworkEvents::handleSpecialMessages(const String& s)
{
    /** Command is first substring */
    String cmd = s.initialSectionNotContaining(" ");

    const MessageManagerLock mmLock;
    if (cmd.compareIgnoreCase ("StartAcquisition") == 0)
    {
        if (! CoreServices::getAcquisitionStatus())
        {
            CoreServices::setAcquisitionStatus (true);
        }
        return String ("StartedAcquisition");
    }
    else if (cmd.compareIgnoreCase ("StopAcquisition") == 0)
    {
        if (CoreServices::getAcquisitionStatus())
        {
            CoreServices::setAcquisitionStatus (false);
        }
        return String ("StoppedAcquisition");
    }
    else if (String ("StartRecord").compareIgnoreCase (cmd) == 0)
    {
        if (! CoreServices::getRecordingStatus())
        {
            /** First set optional parameters (name/value pairs)*/
            if (s.contains ("="))
            {
                String params = s.substring (cmd.length()).trim();
                StringPairArray dict = parseNetworkMessage (params);

                StringArray keys = dict.getAllKeys();
                for (int i = 0; i < keys.size(); ++i)
                {
                    String key   = keys[i];
                    String value = dict[key];

                    if (key.compareIgnoreCase ("CreateNewDir") == 0)
                    {
                        if (value.compareIgnoreCase ("1") == 0)
                        {
                            CoreServices::createNewRecordingDir();
                        }
                    }
                    else if (key.compareIgnoreCase ("RecDir") == 0)
                    {
                        CoreServices::setRecordingDirectory (value);
                    }
                    else if (key.compareIgnoreCase ("PrependText") == 0)
                    {
                        CoreServices::setPrependTextToRecordingDir (value);
                    }
                    else if (key.compareIgnoreCase ("AppendText") == 0)
                    {
                        CoreServices::setAppendTextToRecordingDir (value);
                    }
                }
            }

            /** Start recording */
            CoreServices::setRecordingStatus (true);
            return String ("StartedRecording");
        }
    }
    else if (String ("StopRecord").compareIgnoreCase (cmd) == 0)
    {
        if (CoreServices::getRecordingStatus())
        {
            CoreServices::setRecordingStatus (false);
            return String ("StoppedRecording");
        }
    }
    else if (cmd.compareIgnoreCase ("IsAcquiring") == 0)
    {
        String status = CoreServices::getAcquisitionStatus() ? String ("1") : String ("0");
        return status;
    }
    else if (cmd.compareIgnoreCase ("IsRecording") == 0)
    {
        String status = CoreServices::getRecordingStatus() ? String ("1") : String ("0");
        return status;
    }
    else if (cmd.compareIgnoreCase ("GetRecordingPath") == 0)
    {
        File file = CoreServices::RecordNode::getRecordingPath();
        String msg (file.getFullPathName());
        return msg;
    }
    else if (cmd.compareIgnoreCase ("GetRecordingNumber") == 0)
    {
        String status;
        status += (CoreServices::RecordNode::getRecordingNumber() + 1);
        return status;
    }
    else if (cmd.compareIgnoreCase ("GetExperimentNumber") == 0)
    {
        String status;
        status += CoreServices::RecordNode::getExperimentNumber();
        return status;
    }

    return String ("NotHandled");
}


void NetworkEvents::process (AudioSampleBuffer& buffer)
{
    setTimestampAndSamples(CoreServices::getGlobalTimestamp(),0);

    ScopedLock lock(queueLock);
    while (! networkMessagesQueue.empty())
    {
        const StringTS& msg = networkMessagesQueue.front();
        postTimestamppedStringToMidiBuffer (msg);
        networkMessagesQueue.pop();
    }
}


void NetworkEvents::run()
{
#ifdef ZEROMQ
    HeapBlock<char> buffer(MAX_MESSAGE_LENGTH);

    // responder should always be valid (bound to a port) if it is non-null
    ScopedPointer<Responder> responder(new Responder(0)); // use any available port as default
    if (responder->isValid())
    {
        boundPort = responder->getBoundPort();
    }
    else
    { 
        responder = nullptr;
        boundPort = 0;
    }

    // purposely don't call updatePortString - makeNewSocket will be true on startup,
    // so wait to try the requested port (5556) before updating the editor.

    while (!threadShouldExit())
    {
        // change socket if necessary
        while (makeNewSocket.exchange(false))
        {
            uint16 nextPort = requestedPort; // (maybe the newly entered port on the editor text box)
            if (nextPort > 0 && nextPort == boundPort) // i.e. this is a restart
            {
                responder = nullptr; // destroy old one, which frees the port
                boundPort = 0;
            }

            if (nextPort == 0)
            {
                CoreServices::sendStatusMessage("NetworkEvents: Selecting port automatically");
            }

            ScopedPointer<Responder> newResponder(new Responder(nextPort));
            if (newResponder->isValid())
            {
                // replace the current socket with the newly created socket
                responder = newResponder;
                boundPort = responder->getBoundPort();
            }
            else
            {
                newResponder->reportErr("Failed to connect to port " + String(nextPort));
            }

            updatePortString(boundPort);
        }

        // if we don't have a vaild (connected) socket, keep looping until we do
        if (responder == nullptr)
        {
            wait(100);
            continue;
        }

        int result = responder->receive(buffer);  // times out after RECV_TIMEOUT_MS ms

        juce::int64 timestamp_software = Time::getHighResolutionTicks();

        if (result == -1)
        {
            jassert(responder->getErr() == EAGAIN); // if not, figure out why!
            continue;
        }

        // received message. read string from the buffer.
        String msg = String::fromUTF8(buffer, result);
        {
            ScopedLock lock(queueLock);
            networkMessagesQueue.push({ msg, timestamp_software });
        }

        CoreServices::sendStatusMessage("Network event received: " + msg);
        //std::cout << "Received message!" << std::endl;

        String response = handleSpecialMessages(msg);

        if (responder->send(response) == -1)
        {
            jassertfalse; // figure out why this is failing!
        }
    }

#endif
}


bool NetworkEvents::isReady()
{
    return true;
}


float NetworkEvents::getDefaultSampleRate() const
{
    return 30000.0f;
}


float NetworkEvents::getDefaultBitVolts() const
{
    return 0.05f;
}

void NetworkEvents::setEnabledState (bool newState)
{
    isEnabled = newState;
}


void NetworkEvents::saveCustomParametersToXml (XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement ("NETWORKEVENTS");
    uint16 currBoundPort = boundPort;
    // save the actual bound port if any, otherwise the last attempted port.
    mainNode->setAttribute ("port", currBoundPort ? currBoundPort : requestedPort.load());
}


void NetworkEvents::loadCustomParametersFromXml()
{
    if (parametersAsXml != nullptr)
    {
        forEachXmlChildElement (*parametersAsXml, mainNode)
        {
            if (mainNode->hasTagName ("NETWORKEVENTS"))
            {
                auto port = static_cast<uint16>(mainNode->getIntAttribute("port"));
                if (port != 0)
                {
                    setNewListeningPort(port);
                }
            }
        }
    }
}


StringPairArray NetworkEvents::parseNetworkMessage(StringRef msg)
{
    StringArray args = StringArray::fromTokens(msg, " ", "'\"");
    args.removeEmptyStrings();

    StringPairArray dict;
    for (const String& arg : args)
    {
        int iEq = arg.indexOfChar('=');
        if (iEq >= 0)
        {
            String key = arg.substring(0, iEq);
            String val = arg.substring(iEq + 1).unquoted();
            dict.set(key, val);
        }
    }

    return dict;
}


void NetworkEvents::updatePortString(uint16 port)
{
    auto ed = static_cast<NetworkEventsEditor*>(getEditor());
    if (ed)
    {
        const MessageManagerLock mmLock;
        ed->setPortText(getPortString(port));
    }
}


String NetworkEvents::getEndpoint(uint16 port)
{
    return "tcp://*:" + (port == 0 ? "*" : String(port));
}


String NetworkEvents::getPortString(uint16 port)
{
#ifdef ZEROMQ
    if (port == 0)
    {
        return "<no cxn>";
    }

    return String(port);
#else
    return "<no zeromq>";
#endif
}


/*** ZMQContext ***/

NetworkEvents::ZMQContext::ZMQContext(const ScopedLock& lock)
#ifdef ZEROMQ
    : context(zmq_ctx_new())
#endif
{
    // sharedContextLock should already be held here
    sharedContext = this;
}

// ZMQContext is a ReferenceCountedObject with a pointer in each instance's 
// socket pointer, so this only happens when the last instance is destroyed.
NetworkEvents::ZMQContext::~ZMQContext()
{
    ScopedLock lock(sharedContextLock);
    sharedContext = nullptr;
#ifdef ZEROMQ
    zmq_ctx_destroy(context);
#endif
}

void* NetworkEvents::ZMQContext::createSocket()
{
#ifdef ZEROMQ
    jassert(context != nullptr);
    return zmq_socket(context, ZMQ_REP);
#else
    return nullptr;
#endif
}


/*** Responder ***/

const int NetworkEvents::Responder::RECV_TIMEOUT_MS = 100;

NetworkEvents::Responder::Responder(uint16 port)
    : socket    (nullptr)
    , valid     (false)
    , boundPort (0)
    , lastErrno (0)
{
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
    }

#ifdef ZEROMQ
    socket = context->createSocket();
    if (!socket)
    {
        lastErrno = zmq_errno();
        return;
    }

    // set socket to timeout when receiving rather than blocking forever
    if (zmq_setsockopt(socket, ZMQ_RCVTIMEO, &RECV_TIMEOUT_MS, sizeof(RECV_TIMEOUT_MS)) == -1)
    {
        lastErrno = zmq_errno();
        return;
    }

    // bind to endpoint
    if (zmq_bind(socket, getEndpoint(port).toRawUTF8()) == -1)
    {
        lastErrno = zmq_errno();
        return;
    }

    // if requested port was 0, find out which port was actually used
    if (port == 0)
    {
        const size_t BUF_LEN = 32;
        size_t len = BUF_LEN;
        char endpoint[BUF_LEN];
        if (zmq_getsockopt(socket, ZMQ_LAST_ENDPOINT, endpoint, &len) == -1)
        {
            lastErrno = zmq_errno();
            return;
        }

        port = String(endpoint).getTrailingIntValue();
    }

    jassert(port > 0);
    valid = true;
    boundPort = port;
#endif
}


NetworkEvents::Responder::~Responder()
{
#ifdef ZEROMQ
    if (socket)
    {
        if (boundPort != 0)
        {
            // unbind/disconnect to free the port (critical for restarts)
            zmq_unbind(socket, getEndpoint(boundPort).toRawUTF8());
        }

        int linger = 0;
        zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));
        zmq_close(socket);
    }
#endif
}


int NetworkEvents::Responder::getErr() const
{
    return lastErrno;
}


void NetworkEvents::Responder::reportErr(const String& message) const
{
#ifdef ZEROMQ
    String msg = "NetworkEvents: " + message + " (" + zmq_strerror(lastErrno) + ")";
    std::cout << msg << std::endl;
    CoreServices::sendStatusMessage(msg);
#endif
};


bool NetworkEvents::Responder::isValid() const
{
    return valid;
}


uint16 NetworkEvents::Responder::getBoundPort() const
{
    return boundPort;
}


int NetworkEvents::Responder::receive(void* buf)
{
#ifdef ZEROMQ
    int res = zmq_recv(socket, buf, MAX_MESSAGE_LENGTH, 0);
    if (res == -1)
    {
        lastErrno = zmq_errno();
    }
    else
    {
        res = jmin(res, MAX_MESSAGE_LENGTH);
    }
    return res;
#else
    return -1;
#endif
}


int NetworkEvents::Responder::send(StringRef response)
{
#ifdef ZEROMQ
    int res = zmq_send(socket, response, response.length(), 0);
    if (res == -1)
    {
        lastErrno = zmq_errno();
    }
    return res;
#else
    return -1;
#endif
}
