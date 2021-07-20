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

#include "AudioMonitor.h"
#include "AudioMonitorEditor.h"
#include <stdio.h>


AudioMonitor::AudioMonitor()
    : GenericProcessor ("File Reader")
{
    setProcessorType (PROCESSOR_TYPE_UTILITY);
}


AudioMonitor::~AudioMonitor()
{

}


AudioProcessorEditor* AudioMonitor::createEditor()
{
    editor = new AudioMonitorEditor (this, true);

    return editor;
}

bool AudioMonitor::enable()
{

}

bool AudioMonitor::disable()
{

}

void AudioMonitor::updateSettings()
{

}

void AudioMonitor::process (AudioSampleBuffer& buffer)
{

}


void AudioMonitor::setParameter (int parameterIndex, float newValue)
{

}
