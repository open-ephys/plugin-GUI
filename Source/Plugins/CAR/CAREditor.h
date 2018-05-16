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

#ifndef CAR_EDITOR_H_INCLUDED
#define CAR_EDITOR_H_INCLUDED


#include <EditorHeaders.h>
#include <UIUtilitiesHeaders.h>


class MaterialButtonLookAndFeel;
class ParameterSlider;


/**
   User interface for CAR Processor.

   @see CAR
*/
class CAREditor : public GenericEditor
{
public:
    CAREditor (GenericProcessor* parentProcessor, bool useDefaultParameterEditors);

    // Component methods
    // =========================================================
    void paint (Graphics& g) override;
    void resized()           override;

    // Button::Listener methods
    // ==========================================================
    void buttonClicked (Button* buttonThatWasClicked) override;

    // GenericEditor methods
    // =========================================================
    /** This methods is called when any sliders that we are listen for change their values */
    void sliderEvent (Slider* sliderWhichValueHasChanged) override;
    void channelChanged (int channel, bool newState) override;

    /** Saving/loading parameters */
    void saveCustomParameters(XmlElement* xml) override;
    void loadCustomParameters(XmlElement* xml) override;

private:
    enum ChannelsType
    {
        REFERENCE_CHANNELS = 0,
        AFFECTED_CHANNELS
    };

    ChannelsType m_currentChannelsView;

    ScopedPointer<LinearButtonGroupManager> m_channelSelectorButtonManager;
    ScopedPointer<ParameterSlider>          m_gainSlider;

    // LookAndFeel
    SharedResourcePointer<MaterialButtonLookAndFeel> m_materialButtonLookAndFeel;

    // =========================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CAREditor)
};


#endif  // CAR_H_INCLUDED
