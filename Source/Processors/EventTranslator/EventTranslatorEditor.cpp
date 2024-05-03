/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2022 Open Ephys

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

#include "EventTranslatorEditor.h"
#include "EventTranslator.h"

#include <stdio.h>


EventTranslatorEditor::EventTranslatorEditor (GenericProcessor* parentNode)
    : GenericEditor (parentNode)
{

    desiredWidth = 170;

}

EventTranslatorEditor::~EventTranslatorEditor()
{
}


void EventTranslatorEditor::updateSettings()
{
    
    EventTranslator* proc = (EventTranslator*) getProcessor();
    
    int streamCount = 0;

    Array<ParameterEditor*> toRemove;
    for (auto ed : parameterEditors)
        if (ed->getParameterName() == "sync_line")
            toRemove.add(ed);
    
    for (int i = 0; i < toRemove.size(); i++)
        parameterEditors.removeObject(toRemove[i]);
    
    toRemove.clear();
    
    for (auto stream : proc->getDataStreams())
    {
        
        const uint16 streamId = stream->getStreamId();
        
        int column = streamCount % 5;
        int row = streamCount / 5;
        
        const Array<EventChannel*> eventChannels = proc->getDataStream(streamId)->getEventChannels();

        int numLines;

        if (eventChannels.size() > 0)
            numLines = eventChannels[0]->getMaxTTLBits();
        else
            numLines = 1;

		TtlLineParameter* syncLineParam = (TtlLineParameter*)proc->getDataStream(streamId)->getParameter("sync_line");
        syncLineParam->setMaxAvailableLines(numLines);

		Parameter* mainSyncParam = proc->getParameter("main_sync");
        
		// Add a sync line parameter editor for each stream
        addSyncLineParameterEditor(syncLineParam, (SelectedStreamParameter*)mainSyncParam, 18 + column * 25, 30 + row * 25);
		ParameterEditor* syncEditor = parameterEditors.getLast();
        syncEditor->setSize(18, 18);
        syncEditor->getEditor()->setSize(18, 18);
        syncEditor->disableUpdateOnSelectedStreamChanged();

        streamCount++;
    }
    
}
