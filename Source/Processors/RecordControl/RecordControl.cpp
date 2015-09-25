/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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
#include "RecordControl.h"
#include "../../UI/ControlPanel.h"

RecordControl::RecordControl()
    : GenericProcessor("Record Control"),
      triggerChannel(0)
{

}

RecordControl::~RecordControl()
{

}

AudioProcessorEditor* RecordControl::createEditor()
{
    editor = new RecordControlEditor(this, true);
    return editor;
}

void RecordControl::setParameter(int parameterIndex, float newValue)
{
    if (parameterIndex == 0)
    {
        updateTriggerChannel((int) newValue);
    }
    else if (parameterIndex == 1)
    {
        triggerType = (Types)((int)newValue - 1);
    }
    else if (parameterIndex == 2)
    {
        triggerEdge = (Edges)((int)newValue - 1);
    }
}

void RecordControl::updateTriggerChannel(int newChannel)
{
    triggerChannel = newChannel;
}

bool RecordControl::enable()
{

    return true;
}

void RecordControl::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events)
{
    checkForEvents(events);
}


void RecordControl::handleEvent(int eventType, MidiMessage& event, int)
{
    const uint8* dataptr = event.getRawData();

    int eventId = *(dataptr+2);
    int eventChannel = *(dataptr+3);

    //std::cout << "Received event with id=" << eventId << " and ch=" << eventChannel << std::endl;

    if (eventType == TTL && eventChannel == triggerChannel)
    {
        int edge = triggerEdge == RISING ? 1 : 0;

        //std::cout << "Trigger!" << std::endl;

        const MessageManagerLock mmLock;

        if (triggerType == SET)
        {
            if (eventId == edge)
            {
                CoreServices::setRecordingStatus(true);
            }
            else
            {
                CoreServices::setRecordingStatus(false);
            }
        }
        else if (triggerType == TOGGLE && eventId == edge)
        {
            CoreServices::setRecordingStatus(!CoreServices::getRecordingStatus());
        }
    }
	else if (eventType == MESSAGE)
	{
		handleNetworkEvent(event);
	}

}

void RecordControl::handleNetworkEvent(MidiMessage& event)
{
	/** Extract network message from midi event */
	const uint8* dataptr = event.getRawData();
	int bufferSize = event.getRawDataSize();
    int len = bufferSize - 6; // 6 for initial event prefix
	String msg = String((const char*)(dataptr + 6), len);

	/** Command is first substring */
    StringArray inputs = StringArray::fromTokens(msg, " ");
    String cmd = String(inputs[0]);

    const MessageManagerLock mmLock;

    if (String("StartRecord").compareIgnoreCase(cmd) == 0)
    {
		if (!CoreServices::getRecordingStatus())
		{
			/** First set optional parameters (name/value pairs)*/
		    if (msg.contains("="))
		    {
				String s = msg.substring(cmd.length());
				StringPairArray dict = parseNetworkMessage(s);

				StringArray keys = dict.getAllKeys();
		        for (int i=0; i<keys.size(); i++)
		        {
					String key = keys[i];
					String value = dict[key];

		            if (key.compareIgnoreCase("CreateNewDir") == 0)
		            {
		                if (value.compareIgnoreCase("1") == 0)
		                {
		                    CoreServices::createNewRecordingDir();
		                }
		            }
		            else if (key.compareIgnoreCase("RecDir") == 0)
		            {
		                CoreServices::setRecordingDirectory(value);
		            }
		            else if (key.compareIgnoreCase("PrependText") == 0)
		            {
		                CoreServices::setPrependTextToRecordingDir(value);
		            }
		            else if (key.compareIgnoreCase("AppendText") == 0)
		            {
	                	CoreServices::setAppendTextToRecordingDir(value);
		            }
		        }
		    }

			/** Start recording */
		    CoreServices::setRecordingStatus(true);
		}
    }
    else if (String("StopRecord").compareIgnoreCase(cmd) == 0)
    {
        if (CoreServices::getRecordingStatus())
        {
            CoreServices::setRecordingStatus(false);
        }
    }
}

StringPairArray RecordControl::parseNetworkMessage(String msg)
{
    StringArray splitted;
	splitted.addTokens(msg, "=", "");

	StringPairArray dict = StringPairArray();
	String key = "";
	String value = "";
	for (int i=0; i<splitted.size()-1; i++)
	{
		String s1 = splitted[i];
		String s2 = splitted[i+1];

		/** Get key */
		if (!key.isEmpty())
		{
			if (s1.contains(" "))
			{
				int i1 = s1.lastIndexOf(" ");
				key = s1.substring(i1+1);
			}
			else
			{
				key = s1;
			}
		}
		else
		{
			key = s1.trim();
		}

		/** Get value */
		if (i < splitted.size() - 2)
		{
			int i1 = s2.lastIndexOf(" ");
			value = s2.substring(0, i1);
		}
		else
		{
			value = s2;
		}

		dict.set(key, value);
	}

	return dict;
}

