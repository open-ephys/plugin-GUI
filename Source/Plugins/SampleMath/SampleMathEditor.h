/*
------------------------------------------------------------------

This file is part of a plugin for the Open Ephys GUI
Copyright (C) 2018 Translational NeuroEngineering Laboratory

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

#ifndef SAMPLE_MATH_EDITOR_H_INCLUDED
#define SAMPLE_MATH_EDITOR_H_INCLUDED

#include <EditorHeaders.h>
#include "SampleMath.h"

class SampleMathEditor : public GenericEditor, public ComboBox::Listener, public Label::Listener
{
public:
    SampleMathEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors = false);
    ~SampleMathEditor();

    // implements ComboBox::Listener
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

    // implements Label::Listener
    void labelTextChanged(Label* labelThatHasChanged) override;

    // keep channel selector valid
    void updateSettings() override;

    // catch invalid channel selections
    void channelChanged(int chan, bool newState) override;

    // hide irrelevant control when re-expanded
    void collapsedStateChanged() override;

    void saveCustomParameters(XmlElement* xml) override;
    void loadCustomParameters(XmlElement* xml) override;

private:
    // utility for label listening
    // ouputs whether the label contained a valid input; if so, it is stored in *result.
    static bool updateFloatLabel(Label* labelThatHasChanged,
        float minValue, float maxValue, float defaultValue, float* result);

    // Attempt to parse an input string into a float between min and max, inclusive.
    // Returns false if no float could be parsed.
    static bool parseInput(String& in, float min, float max, float* out);

    // UI elements
    ScopedPointer<ComboBox> operationBox;
    ScopedPointer<ComboBox> useChannelBox;
    ScopedPointer<ComboBox> channelSelectionBox;
    ScopedPointer<Label> constantEditable;

    const String CHANNEL_SELECT_TOOLTIP = "Note: Can only be applied to channels from the same source/subprocessor.";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleMathEditor);
};

#endif // SAMPLE_MATH_EDITOR_H_INCLUDED