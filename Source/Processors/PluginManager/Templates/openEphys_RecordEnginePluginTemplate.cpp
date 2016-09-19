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

PROCESSORHEADERS


PROCESSORCLASSNAME::PROCESSORCLASSNAME()
{
}


PROCESSORCLASSNAME::~PROCESSORCLASSNAME()
{
}


String PROCESSORCLASSNAME::getEngineID() const
{
    return "PROCESSORCLASSNAME";
}


void PROCESSORCLASSNAME::openFiles (File rootFolder, int experimentNumber, int recordingNumber)
{
}


void PROCESSORCLASSNAME::closeFiles()
{
}


void PROCESSORCLASSNAME::writeData  (int writeChannel, int realChannel, const float* buffer, int size)
{
}


void PROCESSORCLASSNAME::writeEvent (int eventType, const MidiMessage& event, int64 timestamp)
{
}


void PROCESSORCLASSNAME::writeSpike (int electrodeIndex, const SpikeObject& spike, int64 timestamp)
{
}


void PROCESSORCLASSNAME::addChannel (int index, const Channel* chan)
{
}


void PROCESSORCLASSNAME::addSpikeElectrode (int index, const SpikeRecordInfo* elec)
{
}


void PROCESSORCLASSNAME::registerProcessor  (const GenericProcessor* processor)
{
}


void PROCESSORCLASSNAME::resetChannels()
{
}


void PROCESSORCLASSNAME::startAcquisition()
{
}


void PROCESSORCLASSNAME::endChannelBlock (bool lastBlock)
{
}


RecordEngineManager* PROCESSORCLASSNAME::getEngineManager()
{
    RecordEngineManager* man = new RecordEngineManager ("PROCESSORCLASSNAME","PLUGINGUINAME", &(engineFactory<PROCESSORCLASSNAME>));
    return man;
}
