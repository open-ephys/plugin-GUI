/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#ifndef __FILEREADER_ACTIONS_H_B327D3D2__
#define __FILEREADER_ACTIONS_H_B327D3D2__

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "FileReader.h"

class SelectFile : public ProcessorAction
{
public:
    SelectFile (FileReader* processor, PathParameter* pathParam, var newValue, CategoricalParameter* activeStream, TimeParameter* startTime, TimeParameter* endTime);

    ~SelectFile() {};

    bool perform() override;

    bool undo() override;

    void restoreOwner (GenericProcessor* p) override;

private:
    FileReader* processor;

    std::string pathKey;
    std::string streamKey;
    std::string startTimeKey;
    std::string endTimeKey;

    int originalActiveStream;

    var originalPath;
    var newPath;

    int originalStartTimeInMs;
    int originalEndTimeInMs;

    int newStartTimeInMs;
    int newEndTimeInMs;
};

#endif // __FILEREADER_ACTIONS_H_B327D3D2__