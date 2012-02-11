/*
  ==============================================================================

    Configuration.h
    Created: 6 Sep 2011 10:24:09pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __CONFIGURATION_H_9DEA9372__
#define __CONFIGURATION_H_9DEA9372__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/GenericProcessor.h"

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
