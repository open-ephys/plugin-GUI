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

#ifndef __SPIKEDETECTOREDITOR_H_F0BD2DD9__
#define __SPIKEDETECTOREDITOR_H_F0BD2DD9__


#include <EditorHeaders.h>

class PopupConfigurationWindow;

/**

  User interface for the SpikeDetector processor.

  Allows the user to add single electrodes, stereotrodes, or tetrodes.

  Parameters of individual channels, such as channel mapping, threshold,
  and enabled state, can be edited.

  @see SpikeDetector

*/

class SpikeDetectorEditor : public GenericEditor,
                            public Button::Listener,
                            public Label::Listener
{
public:
    
    /** Constructor*/
    SpikeDetectorEditor(GenericProcessor* parentNode);

    /** Destructor */
    virtual ~SpikeDetectorEditor() {}

    /** Called when configure button is clicked */
    void buttonClicked(Button* button) override;
    
    /** Called when label text is changed*/
    void labelTextChanged(Label* label) override;

    /** Called when settings are updated*/
    void updateSettings() override;

    /** Adds spike channels with a given type */
    void addSpikeChannels(SpikeChannel::Type type, int count);

    /** Removes a spike channel based on a pointer to a SpikeChannel object*/
    void removeSpikeChannels(Array<SpikeChannel*> spikeChannelsToRemove);

    /** Called when stream is updated */
    void selectedStreamHasChanged() override;
    
private:

    std::unique_ptr<UtilityButton> configureButton;
    
    std::unique_ptr<Label> spikeChannelCountLabel;
    std::unique_ptr<ComboBox> spikeChannelTypeSelector;
    std::unique_ptr<UtilityButton> plusButton;

    PopupConfigurationWindow* currentConfigWindow;
    
    String lastLabelValue;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDetectorEditor);

};




#endif  // __SPIKEDETECTOREDITOR_H_F0BD2DD9__
