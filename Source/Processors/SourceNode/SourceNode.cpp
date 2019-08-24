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

#include "SourceNode.h"
#include "../SourceNode/SourceNodeEditor.h"
#include <stdio.h>
#include "../../AccessClass.h"
#include "../PluginManager/OpenEphysPlugin.h"


SourceNode::SourceNode (const String& name_, DataThreadCreator dt)
    : GenericProcessor      (name_)
    , sourceCheckInterval   (2000)
    , wasDisabled           (true)
    , dataThread            (nullptr)
    , ttlState              (0)
{
    setProcessorType (PROCESSOR_TYPE_SOURCE);

    dataThread = dt (this);

    if (dataThread != nullptr)
    {
        if (! dataThread->foundInputSource())
        {
            setEnabledState (false);
        }
		resizeBuffers();
    }
    else
    {
        setEnabledState (false);
        //   eventChannelState = 0;
    }

    // check for input source every few seconds
    startTimer (sourceCheckInterval);

    timestamp = 0;
}


SourceNode::~SourceNode()
{
    if (dataThread->isThreadRunning())
    {
        std::cout << "Forcing thread to stop." << std::endl;
        dataThread->stopThread (500);
    }
}

bool SourceNode::hasEditor() const
{
	return true;
}

bool SourceNode::isGeneratesTimestamps() const
{
	return true;
}

DataThread* SourceNode::getThread() const
{
	return dataThread;
}

int SourceNode::getTTLState() const
{
	return ttlState;
}

//This is going to be quite slow, since is reallocating everything, but it's the 
//safest way to handle a possible varying number of subprocessors
void SourceNode::resizeBuffers()
{
	inputBuffers.clear();
	eventCodeBuffers.clear();
	eventStates.clear();
	if (dataThread != nullptr)
	{
		dataThread->resizeBuffers();
		int numSubProcs = dataThread->getNumSubProcessors();
		for (int i = 0; i < numSubProcs; i++)
		{
			inputBuffers.add(dataThread->getBufferAddress(i));
			eventCodeBuffers.add(new MemoryBlock(10000*sizeof(uint64)));
			eventStates.add(0);
		}
	}
}


void SourceNode::requestChainUpdate()
{
    CoreServices::updateSignalChain (getEditor());
}


void SourceNode::getEventChannelNames (StringArray& names)
{
    if (dataThread != 0)
        dataThread->getEventChannelNames(names);
}


void SourceNode::updateSettings()
{
	if (dataThread)
	{
		dataThread->updateChannels();
		resizeBuffers();
		int nChans = dataChannelArray.size();
		for (int i = 0; i < nChans; i++)
		{
			String unit = dataThread->getChannelUnits(i);
			if (unit.isNotEmpty())
				dataChannelArray[i]->setDataUnits(unit);
		}
	}
}


void SourceNode::actionListenerCallback (const String& msg)
{
    //std::cout << msg << std::endl;

    if (msg.equalsIgnoreCase ("HI"))
    {
        // std::cout << "HI." << std::endl;
        // dataThread->setOutputHigh();
        ttlState = 1;
    }
    else if (msg.equalsIgnoreCase ("LO"))
    {
        // std::cout << "LO." << std::endl;
        // dataThread->setOutputLow();
        ttlState = 0;
    }
}


float SourceNode::getSampleRate(int sub) const
{
    if (dataThread != nullptr)
        return dataThread->getSampleRate(sub);
    else
        return 44100.0;
}


float SourceNode::getDefaultSampleRate() const
{
    if (dataThread != nullptr)
        return dataThread->getSampleRate(0);
    else
        return 44100.0;
}

int SourceNode::getDefaultNumDataOutputs(DataChannel::DataChannelTypes type, int sub) const
{
	if (dataThread)
		return dataThread->getNumDataOutputs(type, sub);
	else return 0;
}

float SourceNode::getBitVolts (const DataChannel* chan) const
{
    if (dataThread != 0)
        return dataThread->getBitVolts (chan);
    else
        return 1.0f;
}

void SourceNode::setChannelInfo(int channel, String name, float bitVolts)
{
	dataChannelArray[channel]->setName(name);
	dataChannelArray[channel]->setBitVolts(bitVolts);
}

void SourceNode::createEventChannels()
{
	ttlChannels.clear();
	if (dataThread)
	{
		//Create base TTL event channels
		int nSubs = dataThread->getNumSubProcessors();
		for (int i = 0; i < nSubs; i++)
		{
			int nChans = dataThread->getNumTTLOutputs(i);
			nChans = jmin(nChans, 64); //Just 64 TTL channels per source for now
			if (nChans > 0)
			{
				EventChannel* chan = new EventChannel(EventChannel::TTL, nChans, 0, dataThread->getSampleRate(i), this, i);
				chan->setName(getName() + " source TTL events input");
				chan->setDescription("TTL Events coming from the hardware source processor \"" + getName() + "\"");
				chan->setIdentifier("sourceevent");
				eventChannelArray.add(chan);
				ttlChannels.add(chan);
			}
			else
				ttlChannels.add(nullptr);
		}
		//Add other events that the source might create
		Array<EventChannel*> events;
		dataThread->createExtraEvents(events);
		eventChannelArray.addArray(events);
	}
}

void SourceNode::setEnabledState (bool newState)
{
    if (newState && ! dataThread->foundInputSource())
    {
        isEnabled = false;
    }
    else
    {
        isEnabled = newState;
    }
}


void SourceNode::setParameter (int parameterIndex, float newValue)
{
    editor->updateParameterButtons (parameterIndex);
    //std::cout << "Got parameter change notification";
}


AudioProcessorEditor* SourceNode::createEditor()
{
    if (dataThread != nullptr)
    {
        editor = dataThread->createEditor (this);
    }
    else
    {
        editor = nullptr;
    }

    if (editor == nullptr)
    {
        editor = new SourceNodeEditor (this, true);
    }

    return editor;
}


bool SourceNode::tryEnablingEditor()
{
    if (! isSourcePresent())
    {
        //std::cout << "No input source found." << std::endl;
        return false;
    }
    else if (isEnabled)
    {
        // If we're already enabled (e.g. if we're being called again
        // due to timerCallback()), then there's no need to go through
        // the editor again.
        return true;
    }

    std::cout << "Input source found." << std::endl;
    setEnabledState (true);

    GenericEditor* ed = getEditor();
    CoreServices::highlightEditor (ed);
    return true;
}


void SourceNode::timerCallback()
{
    if (! tryEnablingEditor() && isEnabled)
    {
        std::cout << "Input source lost." << std::endl;
        setEnabledState (false);
        GenericEditor* ed = getEditor();
        CoreServices::highlightEditor (ed);
    }
}


bool SourceNode::isReady()
{
    return isSourcePresent() && dataThread->isReady();
}


bool SourceNode::isSourcePresent() const
{
    return dataThread && dataThread->foundInputSource();
}


bool SourceNode::enable()
{
    std::cout << "Source node received enable signal" << std::endl;

    wasDisabled = false;

    stopTimer();

    if (dataThread != nullptr)
    {
        dataThread->startAcquisition();
        return true;
    }
    else
    {
        return false;
    }
}


bool SourceNode::disable()
{
    std::cout << "Source node received disable signal" << std::endl;

    if (dataThread != nullptr)
        dataThread->stopAcquisition();

    startTimer (2000); // timer to check for connected source

    wasDisabled = true;

    std::cout << "SourceNode returning true." << std::endl;

    return true;
}


void SourceNode::acquisitionStopped()
{
    if (! wasDisabled)
    {
        std::cout << "Source node sending signal to UI." << std::endl;

        AccessClass::getUIComponent()->disableCallbacks();
        setEnabledState (false);

        GenericEditor* ed = (GenericEditor*) getEditor();
        CoreServices::highlightEditor (ed);
    }
}

int SourceNode::getNumSubProcessors() const
{
	if (!dataThread) return 0;
	return dataThread->getNumSubProcessors();
}

void SourceNode::process(AudioSampleBuffer& buffer)
{
	int nSubs = dataThread->getNumSubProcessors();
	int copiedChannels = 0;

	for (int sub = 0; sub < nSubs; sub++)
	{
		int channelsToCopy = getNumOutputs(sub);
		
		int nSamples = inputBuffers[sub]->readAllFromBuffer(buffer, &timestamp, static_cast<uint64*>(eventCodeBuffers[sub]->getData()), buffer.getNumSamples(), copiedChannels, channelsToCopy);
		copiedChannels += channelsToCopy;

		setTimestampAndSamples(timestamp, nSamples, sub); 

		if (ttlChannels[sub])
		{
			int numEventChannels = ttlChannels[sub]->getNumChannels();
			// fill event buffer
			uint64 last = eventStates[sub];
			for (int i = 0; i < nSamples; ++i)
			{
				uint64 current = *(static_cast<uint64*>(eventCodeBuffers[sub]->getData()) + i);
				//If there has been no change to the TTL word, avoid doing anything at all here
				if (last != current)
				{
					//Create a TTL event for each bit that has changed
					for (int c = 0; c < numEventChannels; ++c)
					{
						if (((current >> c) & 0x01) != ((last >> c) & 0x01))
						{
							TTLEventPtr event = TTLEvent::createTTLEvent(ttlChannels[sub], timestamp + i, &current, sizeof(uint64), c);
							addEvent(ttlChannels[sub], event, i);
						}
					}
					last = current;
				}
			}
			eventStates.set(sub, last);
		}
	}
}


void SourceNode::saveCustomParametersToXml (XmlElement* parentElement)
{
    XmlElement* channelXml = parentElement->createNewChildElement ("CHANNEL_INFO");
    if (dataThread->usesCustomNames())
    {
        Array<ChannelCustomInfo> channelInfo;
        dataThread->getChannelInfo (channelInfo);
        for (int i = 0; i < channelInfo.size(); ++i)
        {
            XmlElement* chan = channelXml->createNewChildElement ("CHANNEL");
            chan->setAttribute ("name",     channelInfo[i].name);
            chan->setAttribute ("number",   i);
            chan->setAttribute ("gain",     channelInfo[i].gain);
        }
    }
}


void SourceNode::loadCustomParametersFromXml()
{
    if (parametersAsXml != nullptr)
    {
        // use parametersAsXml to restore state
        forEachXmlChildElement (*parametersAsXml, xmlNode)
        {
            if (xmlNode->hasTagName ("CHANNEL_INFO"))
            {
                forEachXmlChildElementWithTagName (*xmlNode, chan, "CHANNEL")
                {
                    const int number = chan->getIntAttribute ("number");
                    const float gain = chan->getDoubleAttribute ("gain");
                    String name = chan->getStringAttribute ("name");

                    dataThread->modifyChannelGain (number, gain);
                    dataThread->modifyChannelName (number, name);
                }
            }
        }
    }
}
