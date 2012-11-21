/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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
	: GenericProcessor("Record Controller"), triggerChannel(0)
{
	
}

RecordControl::~RecordControl()
{

}

AudioProcessorEditor* RecordControl::createEditor()
{
	editor = new RecordControlEditor(this);
	return editor;
}

void RecordControl::updateTriggerChannel(int newChannel)
{
	triggerChannel = newChannel;
}

void RecordControl::process(AudioSampleBuffer &buffer,
							MidiBuffer &events,
							int& nSamples)
{
	checkForEvents(events);
}


void RecordControl::handleEvent(int eventType, MidiMessage& event, int)
{
	uint8* dataptr = event.getRawData();

    int eventId = *(dataptr+2);
    int eventChannel = *(dataptr+3);

	if (eventType == TTL && eventChannel == triggerChannel)
	{

		if (eventId == 1)
			getControlPanel()->setRecordState(true);
		else
			getControlPanel()->setRecordState(false);

	}

}