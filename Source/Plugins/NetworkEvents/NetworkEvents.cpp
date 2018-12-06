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


StringTS::StringTS()
    : timestamp(0)
{}


StringTS::StringTS(String S, int64 ts_software)
    : str(S)
    , timestamp(ts_software)
{}


StringTS::StringTS(MidiMessage& event)
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


std::vector<String> StringTS::splitString (char sep) const
{
    String curr;

    std::list<String> ls;
    for (int k = 0; k < str.length(); ++k)
    {
        if (str[k] != sep)
        {
            curr+=str[k];
        }
        else
        {
            ls.push_back (curr);
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
            ls.push_back (curr);
    }

    std::vector<String> Svec (ls.begin(), ls.end());
    return Svec;
}


/*********************************************/

NetworkEvents::NetworkEvents()
    : GenericProcessor  ("Network Events")
    , Thread            ("NetworkThread")
    , threshold         (200.0)
    , bufferZone        (5.0f)
    , state             (false)
    , zmqcontext        (nullptr)
    , lastGoodPort      (0)
{
    setProcessorType (PROCESSOR_TYPE_SOURCE);

    createZmqContext();

    firstTime = true;
    urlport = 5556;

    opensocket();

    sendSampleCount = false; // disable updating the continuous buffer sample counts,
    // since this processor only sends events
}


void NetworkEvents::setNewListeningPort (uint16 port)
{
    closesocket();
    updatePort(port);
    opensocket();
}


NetworkEvents::~NetworkEvents()
{
    closesocket(true);
}


void NetworkEvents::closesocket(bool shutdown)
{
    std::cout << "Disabling network node" << std::endl;

#ifdef ZEROMQ
    void* oldContext = zmqcontext;
    zmqcontext = nullptr; // this will cause the thread to exit if socket hasn't been created
    zmq_ctx_destroy(oldContext); // this will cause the thread to exit if socket has been created	

    if (!stopThread(500))
    {
        jassertfalse;
        std::cerr << "Network thread timeout. Forcing thread termination, system could be left in an unstable state" << std::endl;
    }

    if (!shutdown)
    {
        createZmqContext(); // so another socket can be opened later
    }
#endif
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


String NetworkEvents::handleSpecialMessages (const String& s)
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
                String params = s.substring (cmd.length());
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
        StringTS msg = networkMessagesQueue.front();
        postTimestamppedStringToMidiBuffer (msg);
        networkMessagesQueue.pop();
    }
}


void NetworkEvents::opensocket()
{
    startThread();
}


void NetworkEvents::run()
{
#ifdef ZEROMQ
    Responder responder(zmqcontext, urlport, lastGoodPort);
    
    uint16 boundPort = responder.getBoundPort();
    if (boundPort == 0)
    {
        // failed to open or bind socket?
        int err = responder.getErr();
        String msg = String("Network Events failed to open socket: ") + zmq_strerror(err);
        std::cout << msg << std::endl;
        if (err != EFAULT && err != ETERM) // errors that could occur normally when context is being terminated
        {
            CoreServices::sendStatusMessage(msg);
        }
        return;
    }
    
    if (urlport != boundPort)
    {
        // update urlport and the editor. this would happen if a new port couldn't be
        // bound to or if 0 was entered, resulting in an ephemeral port being chosen.

        if (urlport == 0)
        {
            CoreServices::sendStatusMessage("Selecting Network Events port automatically");
        }
        else if (boundPort == lastGoodPort)
        {
            CoreServices::sendStatusMessage("Could not bind to port " + String(urlport) + " (" +
                zmq_strerror(responder.getErr()) + "); reverting to port " + String(lastGoodPort));
        }
        else
        {
            jassertfalse;
        }

        const MessageManagerLock mmLock; // to update the editor safely from a thread
        updatePort(boundPort);
    }

    lastGoodPort = urlport;
    char buffer[MAX_MESSAGE_LENGTH];
    int result = -1;

    while (true)
    {
        result = responder.receive(buffer);  // blocking

        juce::int64 timestamp = CoreServices::getGlobalTimestamp();

        if (result < 0)
        {
            // context has been terminated
            break;
        }

        String msgStr = String::fromUTF8(buffer, result);

        {
            StringTS Msg(msgStr, timestamp);
            ScopedLock lock(queueLock);
            networkMessagesQueue.push(Msg);
        }

        CoreServices::sendStatusMessage("Network event received: " + msgStr);

        //std::cout << "Received message!" << std::endl;
        // handle special messages
        String response = handleSpecialMessages (msgStr);
        
        if (responder.send(response) < 0)
        {
            // context has been terminated
            break;
        }
    }

    jassert(responder.getErr() == ETERM);
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
                setNewListeningPort(static_cast<uint16>(mainNode->getIntAttribute("port")));
            }
        }
    }
}


void NetworkEvents::createZmqContext()
{
#ifdef ZEROMQ
    if (zmqcontext == nullptr)
    {
        zmqcontext = zmq_ctx_new(); //<-- this is only available in version 3+
    }
#endif
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


void NetworkEvents::updatePort(uint16 port)
{
    urlport = port;

    auto ed = static_cast<NetworkEventsEditor*>(getEditor());
    if (ed)
    {
        ed->setPortString(String(port));
    }
}


/*** Responder ***/

NetworkEvents::Responder::Responder(void* context, uint16 port, uint16 backupPort)
    : socket        (nullptr)
    , boundPort     (0)
    , lastErrno     (0)
    , initialized   (false)
{
#ifdef ZEROMQ
    socket = zmq_socket(context, ZMQ_REP);
    if (!socket)
    {
        lastErrno = zmq_errno();
        return;
    }

    String url("tcp://*:" + (port == 0 ? "*" : String(port)));
    int rc = zmq_bind(socket, url.toRawUTF8());
    if (rc == -1)
    {
        lastErrno = zmq_errno();

        // try again with last good port, if any
        if (backupPort != 0 && backupPort != port)
        {
            port = backupPort;
            url = "tcp://*:" + String(port);
            if (zmq_bind(socket, url.toRawUTF8()) == -1)
            {
                // don't set errno, since the error for the original port is more relevant
                return;
            }
        }
        else
        {
            return;
        }
    }

    // bound successfully!

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
        jassert(port > 0);
    }

    boundPort = port;
#endif
}


NetworkEvents::Responder::~Responder()
{
#ifdef ZEROMQ
    if (socket)
    {
        int linger = 0;
        zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));
        zmq_close(socket);
    }
#endif
}


int NetworkEvents::Responder::getErr()
{
    int err = lastErrno;
    lastErrno = 0;
    return err;
}


uint16 NetworkEvents::Responder::getBoundPort() const
{
    return boundPort;
}


int NetworkEvents::Responder::receive(void* buf)
{
#ifdef ZEROMQ
    int res = zmq_recv(socket, buf, MAX_MESSAGE_LENGTH, 0); // blocking
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
