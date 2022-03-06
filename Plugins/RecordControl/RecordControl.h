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

#ifndef __RECORDCONTROL_H_120DD434__
#define __RECORDCONTROL_H_120DD434__

#include <ProcessorHeaders.h>
#include "RecordControlEditor.h"


/**
    Stops and stops recording in response to incoming events.

    @see RecordNode
*/
class RecordControl : public GenericProcessor
{
public:

    /** Constructor */
    RecordControl();

    /** Destructor */
    ~RecordControl() { }

    /** Create Record Control Editor*/
    AudioProcessorEditor* createEditor() override;

    /** Call handleEvent() */
    void process (AudioBuffer<float>& buffer) override;

    /** Respond to incoming events */
    void handleEvent (TTLEventPtr event) override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordControl);
};

#endif
