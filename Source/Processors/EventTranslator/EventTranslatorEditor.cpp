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

    desiredWidth = 200;

}

EventTranslatorEditor::~EventTranslatorEditor()
{
}


void EventTranslatorEditor::updateSettings()
{
    /*std::cout << "EventTranslatorEditor::updateSettings()" << std::endl;
    buttons.clear();
    
    EventTranslator* proc = (EventTranslator*) getProcessor();
    
    int streamCount = 0;
    
    for (auto stream : proc->getDataStreams())
    {
        
        std::cout << "Stream: " << stream->getName() << std::endl;
        
        buttons.add(new SyncControlButton(proc,
                                          stream->getName(),
                                          stream->getStreamId()));
        
        buttons.getLast()->setBounds(18 + streamCount * 20, 80, 15, 15);
        addAndMakeVisible(buttons.getLast());

        streamCount++;
    }*/
    
    
}
