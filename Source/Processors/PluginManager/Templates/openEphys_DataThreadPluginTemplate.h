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

#include "../../../JuceLibraryCode/JuceHeader.h"

// TODO <Kirill A> replace this including by using more versatile method
#include "../../Processors/DataThreads/DataThread.h"


class SourceNode;

/**
    This class serves as a template for creating new data thread plugins.

    Fill this comment section to describe the plugin's function.

    @see DataThread
*/
class PROCESSORCLASSNAME : public DataThread
{
public:
    PROCESSORCLASSNAME (SourceNode* sn);
    ~PROCESSORCLASSNAME();

    int getNumHeadstageOutputs() const override;

    float getSampleRate() const override;
    float getBitVolts (Channel* chan) const override;

    bool foundInputSource() override;
    bool startAcquisition() override;
    bool stopAcquisition()  override;


private:
    bool updateBuffer() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PROCESSORCLASSNAME);
};


#endif // HEADERGUARD
