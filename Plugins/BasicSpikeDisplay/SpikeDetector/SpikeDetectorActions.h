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
#include "SpikeDetectorEditor.h"

/**
    Adds a spike channel to the spike detector,
    based on the description.

    Undo: removes the spike channel from the
    spike detector.
*/
class AddSpikeChannels : public OpenEphysAction
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

    void restoreOwner(GenericProcessor* processor) override;

    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();

    XmlElement* settings;

private:

    SpikeDetector* spikeDetector;
    String streamKey;
    SpikeChannel::Type type;
    Array<int> startChannels;

    Array<String> addedSpikeChannels;

    int count;

};

class RemoveSpikeChannels : public OpenEphysAction
{

public:

    /** Constructor*/
    RemoveSpikeChannels(SpikeDetector* processor,
                        DataStream* stream,
                        Array<SpikeChannel*> spikeChannelsToRemove,
                        Array<int> indeces);

    /** Destructor */
    ~RemoveSpikeChannels();

    void restoreOwner(GenericProcessor* processor) override;

    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();

    std::unique_ptr<XmlElement> settings;

private:

    SpikeDetector* spikeDetector;
    String streamKey;
    Array<String> spikeChannelsToRemove;
    Array<SpikeChannel*> removedSpikeChannels;
    Array<int> indeces;

};

#endif /* SpikeDetectorActions_h */
