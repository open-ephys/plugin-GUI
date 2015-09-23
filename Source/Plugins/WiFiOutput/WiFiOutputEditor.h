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

#ifndef __WIFIOUTPUTEDITOR_H_7161DB44__
#define __WIFIOUTPUTEDITOR_H_7161DB44__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../Editors/ImageIcon.h"

class ImageIcon;

/**

  User interface for the WiFiOutput.

  @see WiFiOutput

*/

class WiFiOutputEditor : public GenericEditor

{
public:
    WiFiOutputEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~WiFiOutputEditor();

    void receivedEvent();


    ImageIcon* icon;

private:

    void timerCallback();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WiFiOutputEditor);

};




#endif  // __WIFIOUTPUTEDITOR_H_7161DB44__
