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

#include <EditorHeaders.h>
#include <UIUtilitiesHeaders.h>


class MaterialButtonLookAndFeel;


/**
   User interface for CAR Processor.

   @see CAR
*/
class CAREditor : public GenericEditor
                , private ChannelSelector::Listener
{
public:
    CAREditor (GenericProcessor* parentProcessor, bool useDefaultParameterEditors);

    // Component methods
    // =========================================================
    void resized() override;

    // Button::Listener methods
    // ==========================================================
    void buttonClicked (Button* buttonThatWasClicked) override;

    // ChannelSelector::Listener methods
    // =========================================================
    void channelSelectionChanged (int channel, bool newState) override;


private:
    enum ChannelsType
    {
        REFERENCE_CHANNELS = 0,
        AFFECTED_CHANNELS
    };

    ChannelsType m_currentChannelsView;

    ScopedPointer<ButtonGroupManager> m_channelSelectorButtonManager;

    // LookAndFeel
    SharedResourcePointer<MaterialButtonLookAndFeel> m_materialButtonLookAndFeel;

    // =========================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CAREditor)
};
