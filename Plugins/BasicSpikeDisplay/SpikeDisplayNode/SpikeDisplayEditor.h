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

#ifndef SPIKEDISPLAYEDITOR_H_
#define SPIKEDISPLAYEDITOR_H_

#include <VisualizerEditorHeaders.h>

class Visualizer;
class UtilityButton;

/**

  User interface for the SpikeDisplayNode sink.

  @see SpikeDisplayNode, SpikeDisplayCanvas

*/

#define MAX_N_SUB_CHAN 8

class SpikeDisplayEditor : public VisualizerEditor,
                           public Button::Listener
{
public:

    /** Constructor */
    SpikeDisplayEditor(GenericProcessor*);

    /** Destructor*/
    ~SpikeDisplayEditor() { }

    /** Sends messages from control buttons to canvas (currently disabled) */
    void buttonClicked(Button* button) override;

    /** Creates the SpikeDisplayCanvas */
    Visualizer* createNewCanvas() override;

    /** Writes editor state to xml */
    void saveVisualizerEditorParameters(XmlElement* xml) override;

    /** Writes editor state to xml */
    void loadVisualizerEditorParameters(XmlElement* xml) override;

private:

    std::unique_ptr<UtilityButton> scaleUpBtn;
    std::unique_ptr<UtilityButton> scaleDownBtn;

    std::unique_ptr<Label> scaleLabel;

    Array<float> scaleFactors;
    int selectedScaleFactor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDisplayEditor);

};

#endif  // SPIKEDISPLAYEDITOR_H_
