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

#ifndef PLACEHOLDERPROCESSOR_H_INCLUDED
#define PLACEHOLDERPROCESSOR_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"


class PlaceholderProcessor : public GenericProcessor
{
public:
    PlaceholderProcessor (String pName, String lName, int lVer, bool pSource, bool pSink);
    ~PlaceholderProcessor();

    AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override;

    bool isSource() const override;
    bool isSink()   const override;
    bool isReady()  override;

    void process (AudioSampleBuffer& continuousBuffer) override;


private:
    const String m_processorName;
    const String m_libName;

    const int m_libVersion;

    const bool m_isSourceProcessor;
    const bool m_isSinkProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaceholderProcessor);
};



#endif  // PLACEHOLDERPROCESSOR_H_INCLUDED
