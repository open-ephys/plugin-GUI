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

#include "DataThread.h"


class SourceNode;

/**
    This class serves as a template for creating new data thread plugins.

    Fill this comment section to describe the plugin's function.

    @see DataThread
*/
class PLUGINCLASSNAME : public DataThread
{
public:
    PLUGINCLASSNAME (SourceNode* sn);
    ~PLUGINCLASSMAME();

    float getSampleRate()   const override;
    float getBitVolts()     const override;

    bool foundInputSource() override;
    bool startAcquisition() override;
    bool stopAcquisition()  override;


private:
    bool updateBuffer() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PLUGINCLASSNAME);
};


#endif // HEADERGUARD
