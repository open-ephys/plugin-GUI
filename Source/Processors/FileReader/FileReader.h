/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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


#ifndef __FILEREADER_H_B327D3D2__
#define __FILEREADER_H_B327D3D2__


#include "../../JuceLibraryCode/JuceHeader.h"

#include "GenericProcessor.h"

#define BUFFER_SIZE 102400

/**

  Reads data from a file.

  @see GenericProcessor

*/

class FileReader : public GenericProcessor

{
public:

    FileReader();
    ~FileReader();

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);
    void setParameter(int parameterIndex, float newValue);

    AudioProcessorEditor* createEditor();

    bool hasEditor() const
    {
        return true;
    }

    void updateSettings();

    bool isReady();

    bool isSource()
    {
        return true;
    }

    void enabledState(bool t);

    float getDefaultSampleRate();
    int getDefaultNumOutputs();
    float getDefaultBitVolts();

    void setFile(String fullpath);
    String getFile();

    void saveCustomParametersToXml(XmlElement* parentElement);
    void loadCustomParametersFromXml();

private:

    int64 timestamp;

    int lengthOfInputFile;
    FILE* input;

    int16 readBuffer[BUFFER_SIZE];

    int bufferSize;

    String filePath;

    int counter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileReader);

};


#endif  // __FILEREADER_H_B327D3D2__
