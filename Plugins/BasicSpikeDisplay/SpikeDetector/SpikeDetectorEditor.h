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

class SpikeDetectorEditor : public GenericEditor
{
public:

    /** Constructor*/
    SpikeDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);

    /** Destructor */
    virtual ~SpikeDetectorEditor();

    /** Called when configure button is clicked */
    void buttonEvent(Button* button);

    /** Adds a spike channel with a given type */
    void addSpikeChannel(SpikeChannel::Type type);

    /** Removes a spike channel by index*/
    void removeSpikeChannel(int index);

    /** Called by PopupChannelSelector*/
    void channelStateChanged(Array<int> selectedChannels) override;

private:

    std::unique_ptr<UtilityButton> configureButton;

    PopupConfigurationWindow* currentConfigWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDetectorEditor);

};




#endif  // __SPIKEDETECTOREDITOR_H_F0BD2DD9__
