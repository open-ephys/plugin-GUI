/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#ifndef __CONFIGURATION_H_9DEA9372__
#define __CONFIGURATION_H_9DEA9372__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/GenericProcessor.h"

/**
  
  Holds configuration information for the signal chain.

  The Configuration can be accessed by all processors in order to
     update channel names, electrode groupings, and acquisition parameters.

  @see UIComponent, GenericProcessor

*/

class GenericProcessor;
class Configuration;

class Trode
{
public:
	Trode(int nchans, int startChan, String name_)
	{
		name = name_;

		for (int n = 0; n < nchans; n++)
		{
			channelMapping.add(startChan++);
			inputRange.add(10000.0);
			threshold.add(5000.0);
			isActive.add(true);
		}
	}

	~Trode() {}

	int numChannels() {return channelMapping.size();}

	void setChannel(int wire, int chan)
	{
		if (checkWire(wire))
			channelMapping.set(wire,chan);		
	}

	void setThreshold(int wire, float thresh)
	{
		if (checkWire(wire))
			threshold.set(wire,thresh); 		
	}

	void setInputRange(int wire, float inRange)
	{
		if (checkWire(wire))
			inputRange.set(wire,inRange); 		
	}

	void setState(int wire, bool state)
	{
		if (checkWire(wire))
			isActive.set(wire,state);		
	}

	int getChannel(int wire)
	{
		if (checkWire(wire))
			return channelMapping[wire];
		else 
			return -1;
	}

	float getThreshold(int wire)
	{
		if (checkWire(wire))
			return threshold[wire];
		else 
			return -1;
	}

	float getInputRange(int wire)
	{
		if (checkWire(wire))
			return inputRange[wire];
		else 
			return -1;
	}

	bool getState(int wire)
	{
		if (checkWire(wire))
			return isActive[wire];
		else 
			return false;
	}

	int* getRawDataPointer()
	{
		return channelMapping.getRawDataPointer();
	}

	String getName() {return name;}

private:
	Array<int, CriticalSection> channelMapping;
	Array<float, CriticalSection> threshold;
	Array<float, CriticalSection> inputRange;
	Array<bool, CriticalSection> isActive;

	String name;

	bool checkWire(int wire) 
	{
		if (wire < channelMapping.size() && wire > -1)
			return true;
		else
			return false;	
	}

};

class DataSource
{
public:
	DataSource(GenericProcessor* p, Configuration* c);

	~DataSource() {}

	GenericProcessor* getProcessor() {return processor;}

	int numTetrodes() {return tetrodes.size();}
	int numStereotrodes() {return stereotrodes.size();}
	int numSingleWires() {return singleWires.size();}
	int getElectrodeNumberForChannel(int chan)
	{
		return channelMapping[chan];
	}

	Trode* getTetrode(int n) {return tetrodes[n];}
	Trode* getStereotrode(int n) {return stereotrodes[n];}
	Trode* getSingleWire(int n) {return singleWires[n];}

	void addTrode(int numChannels, String name_)
	{
		Trode* t = new Trode(numChannels, nextAvailableChan, name_);

		for (int n = 0; n < numChannels; n++)
		{
			channelMapping.set(nextAvailableChan++, totalElectrodes);
		}

		totalElectrodes++;

		if (t->numChannels() == 1)
			singleWires.add(t);
		else if (t->numChannels() == 2)
			stereotrodes.add(t);
		else if (t->numChannels() == 4)
			tetrodes.add(t); 
	}

	String getName() {return name;}

	int getNumChans() {return numChans;}
	int getFirstChan() {return firstChan;}

	int id;

private:

	String name;
	
	OwnedArray<Trode, CriticalSection> tetrodes;
	OwnedArray<Trode, CriticalSection> stereotrodes;
	OwnedArray<Trode, CriticalSection> singleWires;
	Array<int, CriticalSection> channelMapping;

	int totalElectrodes;
	int firstChan;
	int numChans;
	int nextAvailableChan;

	GenericProcessor* processor;

};

class Configuration
{
public:
	Configuration() {};
	~Configuration() {};

	void removeDataSource(GenericProcessor*);

	int numDataSources() {return dataSources.size();}
	DataSource* getSource(int sourceNum) {return dataSources[sourceNum];}

	void addDataSource(DataSource* d)
	{
		std::cout << "New data source added." << std::endl;
		dataSources.add(d);
	}

private:

	OwnedArray<DataSource, CriticalSection> dataSources;
};




#endif  // __CONFIGURATION_H_9DEA9372__
