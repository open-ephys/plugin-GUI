/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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


#ifndef __AUDIOMONITOREDITOR_H__
#define __AUDIOMONITOREDITOR_H__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"

/**

  User interface for the "AudioMonitor" source node.

  @see SourceNode, AudioMonitorThread

*/

class AudioMonitorEditor  : public GenericEditor
{
public:
    AudioMonitorEditor (GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~AudioMonitorEditor();

    void buttonEvent (Button* button) override;

    void saveCustomParameters (XmlElement*) override;
    void loadCustomParameters (XmlElement*) override;

	void startAcquisition() override;
	void stopAcquisition()  override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioMonitorEditor);
};



#endif  // __AUDIOMONITOREDITOR_H__
