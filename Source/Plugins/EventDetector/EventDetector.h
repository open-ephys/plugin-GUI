/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef __EVENTDETECTOR_H_91811542__
#define __EVENTDETECTOR_H_91811542__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"

/**

  Searches for threshold crossings and sends out TTL events.

  @see GenericProcessor

*/

class EventDetector : public GenericProcessor

{
public:

    EventDetector();
    ~EventDetector();

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void setParameter(int parameterIndex, float newValue);

private:

    float threshold;
    float bufferZone;
    bool state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EventDetector);

};

#endif  // __EVENTDETECTOR_H_91811542__
