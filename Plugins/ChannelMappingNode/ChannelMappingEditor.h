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

#include <EditorHeaders.h>

#define NUM_REFERENCES 4

/**

  User interface for the Channel Mapping processor.

  @see ChannelMappingNode

*/

class ChannelMappingEditor : public GenericEditor,
    public DragAndDropContainer,
    public Button::Listener

{
public:
    /** Constructor*/
    ChannelMappingEditor(GenericProcessor* parentNode);

    /** Destructor*/
    virtual ~ChannelMappingEditor();

    // Called when an electrode button is clicked
    void buttonClicked(Button* button) override;

    /** Called when the signal chain is updated*/
    void updateSettings() override;

    /** Called when the viewed stream is updated*/
    void selectedStreamHasChanged() override;

    /** Disables channel re-ordering when acquisition is active */
    void startAcquisition() override;

    /** Enables channel re-ordering when acquisition stops*/
    void stopAcquisition() override;

    /** Mouse actions */
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    void mouseDown(const MouseEvent& e) override;

    /** Called when the editor is collapsed or re-opened*/
    void collapsedStateChanged() override;

private:
    
    /** Load settings from .prb JSON */
    void loadPrbFile(File& file);

    /** Write settings to .prb JSON */
    void writePrbFile(File& file);

    /** Updates the electrode button number and location*/
    void refreshElectrodeButtons();

    OwnedArray<ElectrodeButton> electrodeButtons;
   
    std::unique_ptr<LoadButton> loadButton;
    std::unique_ptr<SaveButton> saveButton;
    std::unique_ptr<Viewport> electrodeButtonViewport;
    std::unique_ptr<Component> electrodeButtonHolder;

    bool reorderActive;
    int previousClickedChan;
    int previousShiftClickedChan;
    bool previousClickedState;

    int selectedReferenceIndex;

    bool isDragging;
    int initialDraggedButton;
    int draggingChannel;
    int lastHoverButton;
    bool isConfigured;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelMappingEditor);

};




#endif  // __CHANNELMAPPINGEDITOR_H_73D0AB34__
