/*
 ------------------------------------------------------------------
 
 This file is part of the Open Ephys GUI
 Copyright (C) 2013 Florian Franzen
 
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

#include "RecordEngine.h"
#include "RecordNode.h"
#include "ProcessorGraph.h"

RecordEngine::RecordEngine() {}

RecordEngine::~RecordEngine() {}

void RecordEngine::resetChannels() {}

void RecordEngine::registerProcessor(GenericProcessor* processor) {}

Channel* RecordEngine::getChannel(int index)
{
	return getProcessorGraph()->getRecordNode()->getDataChannel(index);
}

String RecordEngine::generateDateString()
{
	return getProcessorGraph()->getRecordNode()->generateDateString();
}

SpikeRecordInfo* RecordEngine::getSpikeElectrode(int index)
{
	return getProcessorGraph()->getRecordNode()->getSpikeElectrode(index);
}

void RecordEngine::updateTimeStamp(int64 timestamp) {}

void RecordEngine::registerSpikeSource(GenericProcessor* processor) {}

void RecordEngine::startAcquisition() {}

void RecordEngine::directoryChanged() {}