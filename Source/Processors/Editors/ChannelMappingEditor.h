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

#ifndef __CHANNELMAPPINGEDITOR_H_73D0AB34__
#define __CHANNELMAPPINGEDITOR_H_73D0AB34__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

#include "SpikeDetectorEditor.h" // for ElectrodeButton and ElectrodeEditorButton

/**

  User interface for the Channel Mapping processor.

  @see ChannelMappingNode

*/

class ChannelMappingEditor : public GenericEditor

{
public:
    ChannelMappingEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~ChannelMappingEditor();

    void buttonEvent(Button* button);

    void updateSettings();

    void createElectrodeButtons(int numNeeded);

    void saveEditorParameters(XmlElement* xml);
    void loadEditorParameters(XmlElement* xml);

    void channelChanged(int chan);


private:

 	OwnedArray<ElectrodeButton> electrodeButtons;
 	OwnedArray<ElectrodeEditorButton> electrodeEditorButtons;

 	Array<int> channelArray;
 	Array<int> referenceArray;

    int previousChannelCount;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelMappingEditor);

};




#endif  // __CHANNELMAPPINGEDITOR_H_73D0AB34__
