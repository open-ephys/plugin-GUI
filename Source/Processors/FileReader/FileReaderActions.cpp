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

#include "FileReaderActions.h"

SelectFile::SelectFile (FileReader* processor, PathParameter* pathParam, var newValue, CategoricalParameter* activeStream, TimeParameter* startTime, TimeParameter* endTime) 
    : ProcessorAction ("Select File"),
    pathKey (pathParam->getKey()),
    streamKey (activeStream->getKey()),
    startTimeKey (startTime->getKey()),
    endTimeKey (endTime->getKey()),
    processor (processor), 
    originalPath (pathParam->getValue()),
    newPath (newValue),
    originalActiveStream (activeStream->getSelectedIndex()),
    originalStartTimeInMs (startTime->getTimeValue()->getTimeInMilliseconds()), 
    originalEndTimeInMs (endTime->getTimeValue()->getTimeInMilliseconds()) {}

bool SelectFile::perform()
{
    // Get pointers to parameters
    PathParameter* pathParam = static_cast<PathParameter*> (Parameter::parameterMap[pathKey]);
    CategoricalParameter* activeStream = static_cast<CategoricalParameter*> (Parameter::parameterMap[streamKey]);
    TimeParameter* startTime = static_cast<TimeParameter*> (Parameter::parameterMap[startTimeKey]);
    TimeParameter* endTime = static_cast<TimeParameter*> (Parameter::parameterMap[endTimeKey]);

    // Set the new path - this will trigger the linked parameter changes
    pathParam->setNextValue (newPath, false);

    // Load the new file
    processor->setFile (newPath, false);

    // Set the active stream to the first stream
    processor->setActiveStream (0, true);

    // Get duration of the file in total samples
    int64 fileDuration = processor->getCurrentNumTotalSamples();

    // Convert duration to milliseconds
    int64 fileDurationMs = processor->samplesToMilliseconds (fileDuration);

    // Set the maximum time for the time parameters
    endTime->getTimeValue()->setMaxTimeInMilliseconds (int(fileDurationMs));
    startTime->getTimeValue()->setMaxTimeInMilliseconds (int(fileDurationMs - 1));

    // Set the start time to the beginning of the file
    startTime->getTimeValue()->setTimeFromMilliseconds (0);
    startTime->setNextValue (startTime->getTimeValue()->toString(), false);
    processor->setPlaybackStart (0);

    // Set the end time to the end of the file
    endTime->getTimeValue()->setTimeFromMilliseconds (int(fileDurationMs));
    endTime->setNextValue (endTime->getTimeValue()->toString(), false);
    processor->setPlaybackStop (fileDuration);

    // Register the action in case the processor is deleted
    processor->registerUndoableAction (processor->getNodeId(), this);

    return true;
}

bool SelectFile::undo()
{
    // Get pointers to parameters
    PathParameter* pathParam = static_cast<PathParameter*> (Parameter::parameterMap[pathKey]);
    CategoricalParameter* activeStream = static_cast<CategoricalParameter*> (Parameter::parameterMap[streamKey]);
    TimeParameter* startTime = static_cast<TimeParameter*> (Parameter::parameterMap[startTimeKey]);
    TimeParameter* endTime = static_cast<TimeParameter*> (Parameter::parameterMap[endTimeKey]);

    // Set the path to the original path - this will trigger the linked parameter changes
    pathParam->setNextValue (originalPath, false);

    // Load the original file
    processor->setFile (originalPath, false);

    // Set the active stream to the original active stream
    processor->setActiveStream (originalActiveStream, false);

    // Set the start time to the original start time
    startTime->getTimeValue()->setTimeFromMilliseconds (originalStartTimeInMs);
    startTime->setNextValue (startTime->getTimeValue()->toString(), false);
    processor->setPlaybackStart (originalStartTimeInMs * processor->getCurrentSampleRate() / 1000);

    // Set the end time to the original end time
    endTime->getTimeValue()->setTimeFromMilliseconds (originalEndTimeInMs);
    endTime->setNextValue (endTime->getTimeValue()->toString(), false);
    processor->setPlaybackStop (originalEndTimeInMs * processor->getCurrentSampleRate() / 1000);

    return true;
}

void SelectFile::restoreOwner (GenericProcessor* p)
{
    processor = (FileReader*)p;
}
