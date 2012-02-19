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