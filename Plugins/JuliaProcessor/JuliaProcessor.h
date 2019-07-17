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

#ifndef JULIAPROCESSOR_H_INCLUDED
#define JULIAPROCESSOR_H_INCLUDED

#include <ProcessorHeaders.h>

/**
  Julia Processor.

  Allows the user to select a Julia Programming Language file to use as filter

  @see GenericProcessor, JuliaEditor
*/

class JuliaProcessor : public GenericProcessor

{
public:
    JuliaProcessor();
    ~JuliaProcessor();
    void setFile(String fullpath);
    String getFile();
    void reloadFile();
    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void setParameter(int parameterIndex, float newValue);
    void setBuffersize(int bufferSize);
    AudioProcessorEditor* createEditor();
    bool hasEditor() const
    {
        return true;
    }
    void saveCustomParametersToXml(XmlElement* parentElement);
    void loadCustomParametersFromXml();

private:
    bool hasJuliaInstance;
    String filePath;
    int dataHistoryBufferSize;
    int dataHistoryBufferNumChannels; 
    AudioSampleBuffer* dataHistoryBuffer;
    void run_julia_string(String juliaString);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JuliaProcessor);
};


#endif  // JULIAPROCESSOR_H_INCLUDED
