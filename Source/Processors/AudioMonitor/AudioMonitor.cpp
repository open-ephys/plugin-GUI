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
    : GenericProcessor ("Audio Monitor")
{
    setProcessorType (PROCESSOR_TYPE_AUDIO_MONITOR);
}


AudioMonitor::~AudioMonitor()
{

}


AudioProcessorEditor* AudioMonitor::createEditor()
{
    editor = std::make_unique<AudioMonitorEditor>(this, true);

    return editor.get();
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
