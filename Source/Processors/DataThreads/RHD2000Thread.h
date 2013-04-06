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


#ifndef __RHD2000THREAD_H_2C4CBD67__
#define __RHD2000THREAD_H_2C4CBD67__


#include "../../../JuceLibraryCode/JuceHeader.h"

#include <stdio.h>
#include <string.h>

#include "rhythm-api/rhd2000evalboard.h"
#include "rhythm-api/rhd2000registers.h"
#include "rhythm-api/rhd2000datablock.h"
#include "rhythm-api/okFrontPanelDLL.h"

#include "DataThread.h"


class SourceNode;

/**

  Communicates with the RHD2000 Evaluation Board from Intan Technologies

  @see DataThread, SourceNode

*/

class RHD2000Thread : public DataThread

{
public:
    RHD2000Thread(SourceNode* sn);
    ~RHD2000Thread();

    bool foundInputSource();
    int getNumChannels();
    float getSampleRate();
    float getBitVolts();

    bool isHeadstageEnabled(int hsNum);

    void enableHeadstage(int hsNum, bool enabled);
    void setCableLength(int hsNum, float length);
    void setNumChannels(int hsNum, int nChannels);

    int getNumEventChannels();

private:

    ScopedPointer<Rhd2000EvalBoard> evalBoard;
    ScopedPointer<Rhd2000Registers> chipRegisters;
    Rhd2000DataBlock* dataBlock;

    Array<int> numChannelsPerDataStream;

    int numChannels;
    bool deviceFound;

    float thisSample[256];

    int blockSize;

    bool startAcquisition();
    bool stopAcquisition();

    bool updateBuffer();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RHD2000Thread);
};

#endif  // __RHD2000THREAD_H_2C4CBD67__
