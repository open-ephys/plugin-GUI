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


#ifndef __AUDIOMONITOR_H__
#define __AUDIOMONITOR_H__


#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../GenericProcessor/GenericProcessor.h"

/**
  Reads data from a file.

  @see GenericProcessor
*/
class AudioMonitor : public GenericProcessor
{
public:
    AudioMonitor();
    ~AudioMonitor();

    void process (AudioSampleBuffer& buffer) override;
    void setParameter (int parameterIndex, float newValue) override;

    AudioProcessorEditor* createEditor() override;

    bool hasEditor() const  override { return true; }

    void updateSettings() override;

private:


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioMonitor);
};


#endif  // __AUDIOMONITOR_H__
