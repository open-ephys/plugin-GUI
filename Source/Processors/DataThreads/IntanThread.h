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

#ifndef __INTANTHREAD_H_D9135C03__
#define __INTANTHREAD_H_D9135C03__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include <ftdi.h>
#include <stdio.h>
#include "DataThread.h"

class SourceNode;


/**
    Interface to the Intan Demo Board.

    @see SourceNode, DataThread
*/
class IntanThread : public DataThread
{
public:
    IntanThread (SourceNode* sn);
    ~IntanThread();

    bool foundInputSource() override;

    float getSampleRate()   const override;
    float getBitVolts()     const override;

    int getNumEventChannels() const override;

    int getNumChannels() const;


private:
    bool updateBuffer() override;

    bool startAcquisition() override;
    bool stopAcquisition()  override;

    bool initializeUSB (bool);
    bool closeUSB();

    struct ftdi_context ftdic;

    int vendorID, productID;
    int baudrate;

    bool isTransmitting;
    bool deviceFound;

    unsigned char startCode, stopCode;
    unsigned char buffer[240]; // should be 5 samples per channel

    float thisSample[17]; // 17 continuous channels and one event channel

    int ch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IntanThread);
};


#endif  // __INTANTHREAD_H_D9135C03__
