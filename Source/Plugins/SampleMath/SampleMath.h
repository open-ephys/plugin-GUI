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

#ifndef SAMPLE_MATH_H_INCLUDED
#define SAMPLE_MATH_H_INCLUDED

#include <ProcessorHeaders.h>

/*
 * Allows adding, subtracting, multiplying and dividing constants or channels from selected channels.
 *
 * @see GenericProcessor
 */

enum Param
{
    OPERATION,
    USE_CHANNEL,
    CONSTANT,
    CHANNEL
};

// corresponds to indices of ComboBox choices
enum Operation
{
    ADD = 1,
    SUBTRACT,
    MULTIPLY,
    DIVIDE
};

class SampleMath : public GenericProcessor
{
    friend class SampleMathEditor;

public:
    SampleMath();
    ~SampleMath();

    bool hasEditor() const { return true; }
    AudioProcessorEditor* createEditor() override;

    void process(AudioSampleBuffer& continuousBuffer) override;

    void setParameter(int parameterIndex, float newValue) override;

    void updateSettings() override;

private:
    /*
    * Checks whether all active channels are from the same subprocessor as the selected one,
    * if using the selected channel for the operation rather than a constant, and if not deselects
    * those that do not match. Checks each channel against validSubProcFullId.
    */
    void validateActiveChannels();

    // utilities
    juce::uint32 chanToFullID(int chanNum) const;

    // parameters
    Operation operation;
    bool useChannel; // whether operator parameter is a constant or a channel
    float constant;
    int selectedChannel; // -1 = none available
    juce::uint32 validSubProcFullID; // = subproccessor full ID of selected channel

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleMath);
};

#endif // SAMPLE_MATH_H_INCLUDED