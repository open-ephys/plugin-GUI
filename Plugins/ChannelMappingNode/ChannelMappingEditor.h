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

#ifndef __CHANNELMAPPINGEDITOR_H_73D0AB34__
#define __CHANNELMAPPINGEDITOR_H_73D0AB34__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include <EditorHeaders.h>

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

    void createElectrodeButtons(int numNeeded, bool clearPrevious = true);

    void saveCustomParameters(XmlElement* xml);
    void loadCustomParameters(XmlElement* xml);

    void channelChanged (int channel, bool newState) override;

    void mouseDrag(const MouseEvent& e);

    void mouseUp(const MouseEvent& e);

    void mouseDoubleClick(const MouseEvent& e);

    void collapsedStateChanged();

	void startAcquisition();

	int getChannelDisplayNumber(int chan) const override;

    String writePrbFile(File filename);
    String loadPrbFile(File filename);

private:

    void setChannelReference(ElectrodeButton* button);
    void setChannelPosition(int position, int channel);
    void checkUnusedChannels();
    void setConfigured(bool state);

    void refreshButtonLocations();

    OwnedArray<ElectrodeButton> electrodeButtons;
    OwnedArray<ElectrodeButton> referenceButtons;
    ScopedPointer<ElectrodeEditorButton> selectAllButton;
    ScopedPointer<ElectrodeEditorButton> modifyButton;
    ScopedPointer<ElectrodeEditorButton> resetButton;
    ScopedPointer<LoadButton> loadButton;
    ScopedPointer<SaveButton> saveButton;
    ScopedPointer<Viewport> electrodeButtonViewport;
    ScopedPointer<Component> electrodeButtonHolder;

    Array<int> channelArray;
    Array<int> referenceArray;
    Array<int> referenceChannels;
    Array<bool> enabledChannelArray;
	Array<int> channelCountArray;

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
    bool isConfigured;

    ScopedPointer<DynamicObject> info;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelMappingEditor);

};




#endif  // __CHANNELMAPPINGEDITOR_H_73D0AB34__
