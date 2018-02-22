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

class SourceNode;

struct PLUGIN_API ChannelCustomInfo
{
    ChannelCustomInfo()
        : name      ("")
        , gain      (0.f)
        , modified  (false)
    {
    }

    String name;
    float gain;
    bool modified;
};


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
    DataThread (SourceNode* sn);
    ~DataThread();

    /** Calls 'updateBuffer()' continuously while the thread is being run.*/
    void run() override;

    /** Returns the address of the DataBuffer that the input source will fill.*/
    DataBuffer* getBufferAddress(int subProcessor) const;

	/** Called when the chain updates, to add, remove or resize the sourceBuffers' DataBuffers as needed*/
	virtual void resizeBuffers();

    /** Fills the DataBuffer with incoming data. This is the most important
    method for each DataThread.*/
    virtual bool updateBuffer() = 0;

    /** Experimental method used for testing data sources that can deliver outputs.*/
    virtual void setOutputHigh();

    /** Experimental method used for testing data sources that can deliver outputs.*/
    virtual void setOutputLow();

    /** Returns true if the data source is connected, false otherwise.*/
    virtual bool foundInputSource() = 0;

    /** Initializes data transfer.*/
    virtual bool startAcquisition() = 0;

    /** Stops data transfer.*/
    virtual bool stopAcquisition() = 0;

    /** Returns the number of continuous headstage channels the data source can provide.*/
    virtual int getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx) const = 0;

	/** Returns the number of TTL channels that each subprocessor generates*/
	virtual int getNumTTLOutputs(int subProcessorIdx) const = 0;

    /** Returns the sample rate of the data source.*/
    virtual float getSampleRate(int subProcessorIdx) const = 0;

	/** Returns the number of virtual subprocessors this source can generate */
	virtual unsigned int getNumSubProcessors() const;

	/** Called to create extra event channels, apart from the default TTL ones*/
	virtual void createExtraEvents(Array<EventChannel*>& events);

    /** Returns the volts per bit of the data source.*/
    virtual float getBitVolts (const DataChannel* chan) const = 0;

    /** Notifies if the device is ready for acquisition */
    virtual bool isReady();

    virtual int modifyChannelName (int channel, String newName);

    virtual int modifyChannelGain (int channel, float gain);

    /*  virtual void getChannelsInfo(StringArray &Names, Array<ChannelType> &type, Array<int> &stream, Array<int> &originalChannelNumber, Array<float> &gains)
      {
      }*/

    virtual void getEventChannelNames (StringArray& names) const;

    virtual bool usesCustomNames() const;

    /** Changes the names of channels, if the thread needs custom names. */
    void updateChannels();

    /** Returns a pointer to the data input device, in case other processors
    need to communicate with it.*/
  //  virtual void* getDevice();

    void getChannelInfo (Array<ChannelCustomInfo>& infoArray) const;

    /** Create the DataThread custom editor, if any*/
    virtual GenericEditor* createEditor (SourceNode* sn);

	void createTTLChannels();

	virtual String getChannelUnits(int chanIndex) const;

protected:
    virtual void setDefaultChannelNames();

    SourceNode* sn;

    Array<uint64> ttlEventWords;
    Array<int64> timestamps;

    Array<ChannelCustomInfo> channelInfo;
	OwnedArray<DataBuffer> sourceBuffers;

private:
    Time timer;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataThread);
};


#endif  // __DATATHREAD_H_C454F4DB__
