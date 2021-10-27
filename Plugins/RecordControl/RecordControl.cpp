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


#include "RecordControl.h"

RecordControl::RecordControl()
    : GenericProcessor  ("Record Control")
{
    setProcessorType (PROCESSOR_TYPE_UTILITY);

    addCategoricalParameter(Parameter::GLOBAL_SCOPE,
                            "trigger_type",
                            "Determines whether recording state is set or toggled by an incoming event",
                            { "Edge set", "Edge toggle" },
                            0);

    addCategoricalParameter(Parameter::GLOBAL_SCOPE,
                            "edge",
                            "Determines whether recording state is changed by rising or falling events",
                            { "Rising", "Falling" },
                            0);

    addIntParameter(Parameter::STREAM_SCOPE,
                    "trigger_bit",
                    "The TTL bit that triggers a change in recording state",
                    1,
                    1,
                    16);
}


AudioProcessorEditor* RecordControl::createEditor()
{
    editor = std::make_unique<RecordControlEditor> (this);
    return editor.get();
}


void RecordControl::process (AudioSampleBuffer& buffer)
{
    checkForEvents();
}


void RecordControl::handleEvent (const EventChannel* eventInfo, const EventPacket& packet, int)
{

    if (Event::getEventType(packet) == EventChannel::TTL)
    {
        TTLEventPtr ttl = TTLEvent::deserialize (packet, eventInfo);

        uint16 streamId = ttl->getStreamId();

		if (ttl->getBit() == ( int(getParameter(streamId, "Trigger Bit")->getValue()) - 1))
		{
			if (int(getParameter("Trigger Type")->getValue()) == 0)
			{
				if (ttl->getState() == bool(getParameter("Edge")->getValue()))
				{
					CoreServices::setRecordingStatus(true);
				}
				else
				{
					CoreServices::setRecordingStatus(false);
				}
			}
			else
			{
				CoreServices::setRecordingStatus(!CoreServices::getRecordingStatus());
			}
		}
    }
}


