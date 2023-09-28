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

    /**Create & register parameters for the DataThread */
    virtual void registerParameters() {};

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

    /** Called when a parameter value is updated, to allow plugin-specific responses */
    virtual void parameterValueChanged(Parameter*) { }

    // ---------------------
    // NON-VIRTUAL METHODS
    // ---------------------

    /** Calls 'updateBuffer()' continuously while the thread is being run.*/
    void run() override;

    /** Returns the address of the DataBuffer that the input source will fill.*/
    DataBuffer* getBufferAddress(int streamIdx) const;

    /** Adds a boolean parameter, which will later be accessed by name*/
    void addBooleanParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        bool defaultValue,
        bool deactivateDuringAcquisition = false);

    /** Adds an integer parameter, which will later be accessed by name*/
    void addIntParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        int defaultValue,
        int minValue,
        int maxValue,
        bool deactivateDuringAcquisition = false);
    
    /** Adds a string parameter, which will later be accessed by name*/
    void addStringParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        String defaultValue,
        bool deactivateDuringAcquisition = false);

    /** Adds an float parameter, which will later be accessed by name*/
    void addFloatParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        const String& unit,
        float defaultValue,
        float minValue,
        float maxValue,
        float stepSize,
        bool deactivateDuringAcquisition = false);

    /** Adds a selected channels parameter, which will later be accessed by name*/
    void addSelectedChannelsParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        int maxSelectedChannels = std::numeric_limits<int>::max(),
        bool deactivateDuringAcquisition = false);
    
    /** Adds a mask channels parameter, which will later be accessed by name*/
    void addMaskChannelsParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        bool deactivateDuringAcquisition = false);

    /** Adds a categorical parameter, which will later be accessed by name*/
    void addCategoricalParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        Array<String> categories,
        int defaultIndex,
        bool deactivateDuringAcquisition = false);

    /** Adds a path parameter which holds a path to a folder or file */
    void addPathParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        const String& defaultValue,
        const StringArray& validFileExtensions,
        bool isDirectory,
        bool deactivateDuringAcquisition = true);

    /** Adds a selected stream parameter which holds the currentlu selected stream */
    void addSelectedStreamParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        Array<String> streamNames,
        const int defaultIndex,
        bool deactivateDuringAcquisition = true);

    /** Adds a time parameter with microsecond precision in the form HH:MM:SS.sss */
    void addTimeParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        const String& defaultValue = "00:00:00",
        bool deactivateDuringAcquisition = true);

    /** Adds a notification parameter that notifies parameter value changed */
    void addNotificationParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& displayName,
        const String& description,
        bool deactivateDuringAcquisition = false);

    /** Returns a pointer to a parameter with a given name**/
    Parameter* getParameter(String name) const;

	/** Returns true if an object has a parameter with a given name**/
	bool hasParameter(String name) const;

    /** Returns a pointer to a parameter with a given name**/
    Array<Parameter*> getParameters();

protected:

    // ** Allows the DataThread to broadcast a message other plugins */
    void broadcastMessage(String msg);

    SourceNode* sn;

	OwnedArray<DataBuffer> sourceBuffers;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataThread);
};


#endif  // __DATATHREAD_H_C454F4DB__
