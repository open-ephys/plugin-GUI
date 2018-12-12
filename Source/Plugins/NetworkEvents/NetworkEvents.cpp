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
    , threshold         (200.0)
    , bufferZone        (5.0f)
    , state             (false)
    , urlport           (0)
    , firstTime         (true)
    , restart           (false)
    , changeResponder   (false)
{
    setProcessorType (PROCESSOR_TYPE_SOURCE);

    portString = getPortString();
    portString.addListener(this);

    if (!setNewListeningPort(5556))
    {
        // resort to choosing a port automatically
        setNewListeningPort(0);
    }

    sendSampleCount = false; // disable updating the continuous buffer sample counts,
    // since this processor only sends events

    startThread();
}


NetworkEvents::~NetworkEvents()
{
    if (!stopThread(1000))
    {
        jassertfalse; // shouldn't block for more than 100 ms, something's wrong
        std::cerr << "Network thread timeout. Forcing thread termination, system could be left in an unstable state" << std::endl;
    }
}


bool NetworkEvents::setNewListeningPort(uint16 port)
{
    ScopedPointer<Responder> newResponder = Responder::makeResponder(port);
    if (newResponder == nullptr)
    {
        return false;
    }

    ScopedLock nrLock(nextResponderLock);
    nextResponder = newResponder;
    changeResponder = true;
    return true;
}


String NetworkEvents::getPortString() const
{
#ifdef ZEROMQ
    uint16 port = urlport;
    if (port == 0)
    {
        return "<no cxn>";
    }

    return String(port);
#else
    return "<no zeromq>";
#endif
}


void NetworkEvents::restartConnection()
{
    restart = true;
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


void NetworkEvents::setParameter (int parameterIndex, float newValue)
{
    /*
       editor->updateParameterButtons(parameterIndex);

       Parameter& p =  parameters.getReference(parameterIndex);
       p.setValue(newValue, 0);

       threshold = newValue;
    */
    //std::cout << float(p[0]) << std::endl;
}


void NetworkEvents::initSimulation()
{
    Time t;

    const int64 secondsToTicks = t.getHighResolutionTicksPerSecond();
    simulationStartTime = 3 * secondsToTicks + t.getHighResolutionTicks(); // start 10 seconds after

    simulation.push (StringTS ("ClearDesign",    simulationStartTime));
    simulation.push (StringTS ("NewDesign Test", simulationStartTime + 0.5 * secondsToTicks));
    simulation.push (StringTS ("AddCondition Name GoRight TrialTypes 1 2 3", simulationStartTime + 0.6 * secondsToTicks));
    simulation.push (StringTS ("AddCondition Name GoLeft TrialTypes 4 5 6",  simulationStartTime + 0.6 * secondsToTicks));
}


void NetworkEvents::simulateDesignAndTrials ()
{
    Time t;
    while (simulation.size() > 0)
    {
        int64 currenttime = t.getHighResolutionTicks();
        StringTS S = simulation.front();
        if (currenttime > S.timestamp)
        {
            // handle special messages
            handleSpecialMessages (S.str);

            postTimestamppedStringToMidiBuffer (S);
            //getUIComponent()->getLogWindow()->addLineToLog(S.getString());
            simulation.pop();
        }
        else
        {
            break;
        }
    }

}

void NetworkEvents::postTimestamppedStringToMidiBuffer (StringTS s)
{
	MetaDataValueArray md;
	md.add(new MetaDataValue(MetaDataDescriptor::INT64, 1, &s.timestamp));
	TextEventPtr event = TextEvent::createTextEvent(messageChannel, CoreServices::getGlobalTimestamp(), s.str, md);
	addEvent(messageChannel, event, 0);
}


void NetworkEvents::simulateStopRecord()
{
    Time t;
    simulation.push (StringTS ("StopRecord", t.getHighResolutionTicks()));
}


void NetworkEvents::simulateStartRecord()
{
    Time t;
    simulation.push (StringTS ("StartRecord", t.getHighResolutionTicks()));
}


void NetworkEvents::simulateSingleTrial()
{
    std::cout << "Simulating trial." << std::endl;

    const int numTrials = 1;
    const float ITI         = 0.7;
    const float TrialLength = 0.4;

    Time t;

    if (firstTime)
    {
        firstTime = false;
        initSimulation();
    }

    int64 secondsToTicks = t.getHighResolutionTicksPerSecond();
    simulationStartTime = 3 * secondsToTicks + t.getHighResolutionTicks(); // start 10 seconds after

    // trial every 5 seconds
    for (int k = 0; k < numTrials; ++k)
    {
        simulation.push (StringTS ("TrialStart", simulationStartTime + ITI * k * secondsToTicks));

        if (k % 2 == 0)
            // 100 ms after trial start
            simulation.push (StringTS ("TrialType 2", simulationStartTime + (ITI * k + 0.1) * secondsToTicks));
        else
            // 100 ms after trial start
            simulation.push (StringTS ("TrialType 4", simulationStartTime + (ITI * k + 0.1) * secondsToTicks));

        // 100 ms after trial start
        simulation.push (StringTS ("TrialAlign",     simulationStartTime + (ITI * k + 0.1)         * secondsToTicks));
        // 300 ms after trial start
        simulation.push (StringTS ("TrialOutcome 1", simulationStartTime + (ITI * k + 0.3)         * secondsToTicks));
        // 400 ms after trial start
        simulation.push (StringTS ("TrialEnd",       simulationStartTime + (ITI * k + TrialLength) * secondsToTicks));
    }
}


String NetworkEvents::handleSpecialMessages(const String& s)
{
    /*
    std::vector<String> input = msg.splitString(' ');
    if (input[0] == "StartRecord")
    {
    getUIComponent()->getLogWindow()->addLineToLog("Remote triggered start recording");

    if (input.size() > 1)
    {
    getUIComponent()->getLogWindow()->addLineToLog("Remote setting session name to "+input[1]);
    // session name was also given.
    getProcessorGraph()->getRecordNode()->setDirectoryName(input[1]);
    }
    const MessageManagerLock mmLock;
    getControlPanel()->recordButton->setToggleState(true,true);
    return String("OK");
    //	getControlPanel()->placeMessageInQueue("StartRecord");
    } if (input[0] == "SetSessionName")
    {
    getProcessorGraph()->getRecordNode()->setDirectoryName(input[1]);
    } else if (input[0] == "StopRecord")
    {
    const MessageManagerLock mmLock;
    //getControlPanel()->placeMessageInQueue("StopRecord");
    getControlPanel()->recordButton->setToggleState(false,true);
    return String("OK");
    } else if (input[0] == "ProcessorCommunication")
    {
    ProcessorGraph *g = getProcessorGraph();
    Array<GenericProcessor*> p = g->getListOfProcessors();
    for (int k=0;k<p.size();k++)
    {
    if (p[k]->getName().toLowerCase() == input[1].toLowerCase())
    {
    String Query="";
    for (int i=2;i<input.size();i++)
    {
    if (i == input.size()-1)
    Query+=input[i];
    else
    Query+=input[i]+" ";
    }

    return p[k]->interProcessorCommunication(Query);
    }
    }

    return String("OK");
    }

    */

    /** Command is first substring */
    String cmd = s.initialSectionNotContaining(" ");
    String params = s.substring(cmd.length()).trim();

    const MessageManagerLock mmLock;
    if (cmd.equalsIgnoreCase("StartAcquisition"))
    {
        CoreServices::setAcquisitionStatus(true);
        return "StartedAcquisition";
    }
    else if (cmd.equalsIgnoreCase("StopAcquisition"))
    {
        CoreServices::setAcquisitionStatus(false);
        return "StoppedAcquisition";
    }
    else if (cmd.equalsIgnoreCase("StartRecord"))
    {
        if (!CoreServices::getRecordingStatus())
        {
            /** First set optional parameters (name/value pairs)*/
            StringPairArray dict = parseNetworkMessage(params);

            StringArray keys = dict.getAllKeys();
            for (int i = 0; i < keys.size(); ++i)
            {
                String key = keys[i];
                String value = dict[key];

                if (key.equalsIgnoreCase("CreateNewDir"))
                {
                    if (value.equalsIgnoreCase("1"))
                    {
                        CoreServices::createNewRecordingDir();
                    }
                }
                else if (key.equalsIgnoreCase("RecDir"))
                {
                    CoreServices::setRecordingDirectory(value);
                }
                else if (key.equalsIgnoreCase("PrependText"))
                {
                    CoreServices::setPrependTextToRecordingDir(value);
                }
                else if (key.equalsIgnoreCase("AppendText"))
                {
                    CoreServices::setAppendTextToRecordingDir(value);
                }
            }

            /** Start recording */
            CoreServices::setRecordingStatus(true);
            return "StartedRecording";
        }
    }
    else if (cmd.equalsIgnoreCase("StopRecord"))
    {
        CoreServices::setRecordingStatus(false);
        return "StoppedRecording";
    }
    else if (cmd.equalsIgnoreCase("IsAcquiring"))
    {
        return CoreServices::getAcquisitionStatus() ? String("1") : String("0");
    }
    else if (cmd.equalsIgnoreCase("IsRecording"))
    {
        return CoreServices::getRecordingStatus() ? String("1") : String("0");
    }
    else if (cmd.equalsIgnoreCase("GetRecordingPath"))
    {
        File file = CoreServices::RecordNode::getRecordingPath();
        return file.getFullPathName();
    }
    else if (cmd.equalsIgnoreCase("GetRecordingNumber"))
    {
        return String(CoreServices::RecordNode::getRecordingNumber() + 1);
    }
    else if (cmd.equalsIgnoreCase("GetExperimentNumber"))
    {
        return String(CoreServices::RecordNode::getExperimentNumber());
    }

    return "NotHandled";
}


void NetworkEvents::process (AudioSampleBuffer& buffer)
{
    setTimestampAndSamples(CoreServices::getGlobalTimestamp(),0);

    ScopedLock lock(queueLock);
    while (! networkMessagesQueue.empty())
    {
        StringTS msg = networkMessagesQueue.front();
        postTimestamppedStringToMidiBuffer (msg);
        networkMessagesQueue.pop();
    }
}


void NetworkEvents::run()
{
#ifdef ZEROMQ
    ScopedPointer<Responder> responder;
    char buffer[MAX_MESSAGE_LENGTH];
    bool connected = false;

    while (!threadShouldExit())
    {
        // reopen connection if necessary
        if (restart)
        {
            restart = false; // the other thread only sets restart to true, so it's not a race condition
            responder = nullptr; // destroy old one, which frees the port
            responder = Responder::makeResponder(urlport);

            if (responder != nullptr)
            {
                connected = true;
                updatePort(responder->getBoundPort());
            }
        }

        // switch to new responder if necessary
        if (changeResponder)
        {
            const ScopedLock nrLock(nextResponderLock);
            if (nextResponder != nullptr)
            {
                changeResponder = false; // this is also controlled by the nextResponderLock
                responder = nextResponder;
                connected = true;
                updatePort(responder->getBoundPort());
            }
            else
            {
                jassertfalse; // huh? a new responder should be available
            }
        }

        // if we don't have a vaild (connected) socket, keep looping until we do
        if (responder == nullptr)
        {
            if (connected)
            {
                connected = false;
                updatePort(0);
            }
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
        String msgStr = String::fromUTF8(buffer, result);

        {
            StringTS Msg(msgStr, timestamp_software);
            ScopedLock lock(queueLock);
            networkMessagesQueue.push(Msg);
        }

        CoreServices::sendStatusMessage("Network event received: " + msgStr);
        //std::cout << "Received message!" << std::endl;

        String response = handleSpecialMessages(msgStr);

        if (responder->send(response) == -1)
        {
            jassertfalse; // figure out why this is failing!
        }
    }

#endif
}


int NetworkEvents::getDefaultNumOutputs() const
{
    return 0;
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
    mainNode->setAttribute ("port", urlport);
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


void NetworkEvents::valueChanged(Value& value)
{
    if (value.refersToSameSourceAs(portString))
    {
        auto ed = static_cast<NetworkEventsEditor*>(getEditor());
        if (ed)
        {
            ed->setPortText(value.toString());
        }
    }
}


void NetworkEvents::updatePort(uint16 port)
{
    urlport = port;
    portString = getPortString();
}


String NetworkEvents::getEndpoint(uint16 port)
{
    return "tcp://*:" + (port == 0 ? "*" : String(port));
}


/*** StringTS ***/

NetworkEvents::StringTS::StringTS()
    : timestamp(0)
{}


NetworkEvents::StringTS::StringTS(String S, int64 ts_software)
    : str(S)
    , timestamp(ts_software)
{}


NetworkEvents::StringTS::StringTS(MidiMessage& event)
    : timestamp(EventBase::getTimestamp(event))
{
    if (Event::getEventType(event) != EventChannel::EventChannelTypes::TEXT)
    {
        return; // only handles text events
    }

    const uint8* dataptr = event.getRawData();
    // relying on null terminator to get end of string...
    str = String::fromUTF8(reinterpret_cast<const char*>(dataptr + EVENT_BASE_SIZE));
}


std::vector<String> NetworkEvents::StringTS::splitString(char sep) const
{
    String curr;

    std::list<String> ls;
    for (int k = 0; k < str.length(); ++k)
    {
        if (str[k] != sep)
        {
            curr += str[k];
        }
        else
        {
            ls.push_back(curr);
            while (str[k] == sep && k < str.length())
                ++k;

            curr = "";
            if (str[k] != sep && k < str.length())
                curr += str[k];
        }
    }
    if (str.length() > 0)
    {
        if (str[str.length() - 1] != sep)
            ls.push_back(curr);
    }

    std::vector<String> Svec(ls.begin(), ls.end());
    return Svec;
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
    : socket(nullptr)
    , boundPort(0)
    , lastErrno(0)
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
        CoreServices::sendStatusMessage("NetworkEvents: Selecting port automatically");
        const size_t BUF_LEN = 32;
        size_t len = BUF_LEN;
        char endpoint[BUF_LEN];
        if (zmq_getsockopt(socket, ZMQ_LAST_ENDPOINT, endpoint, &len) == -1)
        {
            lastErrno = zmq_errno();
            return;
        }

        port = String(endpoint).getTrailingIntValue();
        jassert(port > 0);
    }

    boundPort = port;
#endif
}


ScopedPointer<NetworkEvents::Responder> NetworkEvents::Responder::makeResponder(uint16 port)
{
    ScopedPointer<Responder> socketPtr = new Responder(port);
    uint16 boundPort = socketPtr->getBoundPort();
    if (boundPort == 0)
    {
        socketPtr->reportErr("Failed to connect to port " + String(port));
        return nullptr;
    }

    if (port != 0 && boundPort != port)
    {
        jassertfalse; // huh?
        return nullptr;
    }
    return socketPtr;
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
