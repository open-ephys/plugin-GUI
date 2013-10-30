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

#include "ElectrodeButtons.h" // for ElectrodeButton and ElectrodeEditorButton

#define NUM_REFERENCES 4

/**

  User interface for the Channel Mapping processor.

  @see ChannelMappingNode

*/

class ChannelMappingEditor : public GenericEditor,
	public DragAndDropContainer

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

	void mouseDrag(const MouseEvent &e);

	void mouseUp(const MouseEvent &e);

	void mouseDoubleClick(const MouseEvent &e);


private:

	void setChannelReference(ElectrodeButton *button);
	void setChannelPosition(int position, int channel);

	OwnedArray<ElectrodeButton> electrodeButtons;
	OwnedArray<ElectrodeButton> referenceButtons;
    ScopedPointer<ElectrodeEditorButton> selectAllButton;
    ScopedPointer<ElectrodeEditorButton> modifyButton;

    Array<int> channelArray;
    Array<int> referenceArray;
	Array<int> referenceChannels;
	Array<bool> enabledChannelArray;

    int previousChannelCount;
	int selectedReference;
	bool reorderActive;
	int previousClickedChan;
	int previousShiftClickedChan;
	bool previousClickedState;

	bool isDragging;
	int initialDraggedButton;
	int draggingChannel;
	int lastHoverButton;
	

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelMappingEditor);

};




#endif  // __CHANNELMAPPINGEDITOR_H_73D0AB34__
