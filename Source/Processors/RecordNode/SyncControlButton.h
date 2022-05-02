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

#ifndef SyncControlButton_h
#define SyncControlButton_h

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../../Utils/Utils.h"

class SynchronizingProcessor;

/**
    
    Selects a sync line for a particular data stream
 
 */
class SyncControlButton :
    public Button,
    public Timer,
    public ComponentListener
{
public:
    
    /** Constructor */
    SyncControlButton(SynchronizingProcessor* node,
                      const String& name,
                      uint16 streamId,
                      int ttlLineCount = 8);
    
    /** Destructor */
    ~SyncControlButton();

    /** Creates the sync selection interface */
    void mouseUp(const MouseEvent &event) override;
    
    int streamId;
    bool isPrimary;
    int ttlLineCount;

private:
    
    /** Checks whether the underlying stream is synchronized */
    void timerCallback();
    
    /** Renders the button */
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
    
    /** Called when popup selection interface is closed */
    void componentBeingDeleted(Component &component);
    
    SynchronizingProcessor* node;

};


#endif /* SyncControlButton_h */
