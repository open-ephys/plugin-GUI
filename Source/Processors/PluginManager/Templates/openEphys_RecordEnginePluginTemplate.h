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

#ifndef HEADERGUARD
#define HEADERGUARD

#include <RecordingLib.h>


class PROCESSORCLASSNAME : public RecordEngine
{
public:
    PROCESSORCLASSNAME();
    ~PROCESSORCLASSNAME();

    String getEngineID() const override;

    void openFiles (File rootFolder, int experimentNumber, int recordingNumber) override;
    void closeFiles() override;

    void writeData  (int writeChannel, int realChannel, const float* buffer, int size)  override;
    void writeEvent (int eventType, const MidiMessage& event, int64 timestamp)          override;
    void writeSpike (int electrodeIndex, const SpikeObject& spike, int64 timestamp)     override;

    void addChannel         (int index, const Channel* chan)            override;
    void addSpikeElectrode  (int index, const SpikeRecordInfo* elec)    override;

    void registerProcessor  (const GenericProcessor* processor) override;

    void resetChannels()    override;
    void startAcquisition() override;

    void endChannelBlock (bool lastBlock) override;

    static RecordEngineManager* getEngineManager();


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PROCESSORCLASSNAME);
};

#endif // HEADERGUARD
