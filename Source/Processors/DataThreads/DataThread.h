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

#ifndef __DATATHREAD_H_C454F4DB__
#define __DATATHREAD_H_C454F4DB__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>
#include "DataBuffer.h"
#include "../GenericProcessor/GenericProcessor.h"

#include "../Settings/DataStream.h"
#include "../Settings/EventChannel.h"
#include "../Settings/ContinuousChannel.h"
#include "../Settings/SpikeChannel.h"
#include "../Settings/ConfigurationObject.h"
#include "../Settings/DeviceInfo.h"
#include "../Settings/InfoObject.h"

class SourceNode;

/**
    Abstract base class for a data input thread owned by the SourceNode.

    To communicate with input sources that may have a different clock as the
    data acquisition callbacks, it's most efficient to use a separate thread.
    The DataThread class makes it easy to create threads that interact with
    new data sources, such as an FPGA, an Arduino, or a network stream.

    @see SourceNode
*/

class PLUGIN_API DataThread : public Thread
{
public:

    /** Constructor */
    DataThread (SourceNode* sn);

    /** Destructor*/
    ~DataThread();

    // ---------------------
    // PURE VIRTUAL METHODS
    // ---------------------

    /** Fills the DataBuffer with incoming data. This is the most important
        method for each DataThread.*/
    virtual bool updateBuffer() = 0;

    /** Returns true if the data source is connected, false otherwise.*/
    virtual bool foundInputSource() = 0;

    /** Initializes data transfer.*/
    virtual bool startAcquisition() = 0;

    /** Stops data transfer.*/
    virtual bool stopAcquisition() = 0;

    /* Passes the processor's info objects to DataThread, to allow them to be configured */
    virtual void updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
        OwnedArray<EventChannel>* eventChannels,
        OwnedArray<SpikeChannel>* spikeChannels,
        OwnedArray<DataStream>* sourceStreams,
        OwnedArray<DeviceInfo>* devices,
        OwnedArray<ConfigurationObject>* configurationObjects) = 0;

    // ---------------------
    // VIRTUAL METHODS
    // ---------------------

    /** Called when the chain updates, to add, remove or resize the sourceBuffers' DataBuffers as needed*/
    virtual void resizeBuffers() { }

    /** Create the DataThread custom editor, if any*/
    virtual std::unique_ptr<GenericEditor> createEditor(SourceNode* sn);

    /** Allows the DataThread plugin to respond to broadcast messages sent by other processors
          during acquisition */
    virtual void handleBroadcastMessage(String msg) { }

    // ** Allows the DataThread plugin to handle a config message while acquisition is NOT active. */
    virtual String handleConfigMessage(String msg) { return ""; }

    /** Allows the DataThread to set its default state, depending on whether the signal chain is loading */
    virtual void initialize(bool signalChainIsLoading) { }

    // ---------------------
    // NON-VIRTUAL METHODS
    // ---------------------

    /** Calls 'updateBuffer()' continuously while the thread is being run.*/
    void run() override;

    /** Returns the address of the DataBuffer that the input source will fill.*/
    DataBuffer* getBufferAddress(int streamIdx) const;

protected:

    // ** Allows the DataThread to broadcast a message other plugins */
    void broadcastMessage(String msg);

    SourceNode* sn;

	OwnedArray<DataBuffer> sourceBuffers;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataThread);
};


#endif  // __DATATHREAD_H_C454F4DB__
