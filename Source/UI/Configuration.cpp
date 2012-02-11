/*
  ==============================================================================

    Configuration.cpp
    Created: 6 Sep 2011 10:24:09pm
    Author:  jsiegle

  ==============================================================================
*/

#include "Configuration.h"


DataSource::DataSource(GenericProcessor* p, Configuration* c)
{

	processor = p;

	name = p->getName();
	id = p->getNodeId();

	firstChan = 0;

	for (int n = 0; n < c->numDataSources(); n++)
	{
       firstChan += c->getSource(n)->getNumChans();
	}

	numChans = p->getNumOutputs();

	nextAvailableChan = firstChan;

	channelMapping.ensureStorageAllocated(numChans);

	totalElectrodes = 0;

}


void Configuration::removeDataSource(GenericProcessor* p)
{
	std::cout << "Removing data source from configuration!" << std::endl;

	for (int n = 0; n < numDataSources(); n++) 
	{
		GenericProcessor* gp = (GenericProcessor*) getSource(n)->getProcessor();
		
		if (gp == p)
			dataSources.remove(n);
	}

}