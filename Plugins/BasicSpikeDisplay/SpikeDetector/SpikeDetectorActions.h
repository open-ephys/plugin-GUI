/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2023 Open Ephys

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

#ifndef SpikeDetectorActions_h
#define SpikeDetectorActions_h

#include <ProcessorHeaders.h>

#include "SpikeDetector.h"

/**
    Adds a spike channel to the spike detector,
    based on the description.

    Undo: removes the spike channel from the
    spike detector.
*/
class AddSpikeChannels : public UndoableAction
{

public:

    /** Constructor*/
    AddSpikeChannels(SpikeDetector* processor,
                    DataStream* stream,
                    SpikeChannel::Type type,
                    int count, //adds multiple channels atonce
                    Array<int> startChannels);

    /** Destructor */
    ~AddSpikeChannels();

    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();

    XmlElement* settings;

private:

    SpikeDetector* processor;
    uint16 streamId;
    SpikeChannel::Type type;
    Array<int> startChannels;

    int count;

};

class RemoveSpikeChannels : public UndoableAction
{

public:

    /** Constructor*/
    RemoveSpikeChannels(SpikeDetector* processor,
                        DataStream* stream,
                        Array<SpikeChannel*> spikeChannelsToRemove);

    /** Destructor */
    ~RemoveSpikeChannels();

    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();

    std::unique_ptr<XmlElement> settings;

private:

    SpikeDetector* processor;
    uint16 streamId;
    Array<SpikeChannel*> spikeChannelsToRemove;
    Array<SpikeChannel*> removedSpikeChannels;

};

#endif /* SpikeDetectorActions_h */
