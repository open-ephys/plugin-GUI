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

#include "ScrubberInterface.h"

void FullTimeline::paint (Graphics& g)
{
    /* Draw timeline background */
    int tickHeight = 4;
    int borderThickness = 1;
    g.setColour (findColour (ThemeColours::componentParentBackground));

    int numTicks = 5;

    for (int i = 0; i < numTicks; i++)
    {
        float dX = float (i) / numTicks * 450;

        Path tick;
        tick.startNewSubPath (dX, this->getHeight() - tickHeight);
        tick.lineTo (dX, this->getHeight());
        g.strokePath (tick, PathStrokeType (1.0));
    }

    g.fillRect (0, 0, this->getWidth(), this->getHeight() - tickHeight);
    g.setColour (findColour (ThemeColours::widgetBackground));
    g.fillRect (borderThickness, borderThickness, this->getWidth() - 2 * borderThickness, this->getHeight() - 2 * borderThickness - tickHeight);

    /* Draw a coloured vertical bar for each event */

    float sampleRate = fileReader->getCurrentSampleRate();
    int64 totalSamples = (stopMs - startMs) / 1000.0f * sampleRate;

    int64 startSample = startMs / 1000.0f * sampleRate;
    int64 stopSample = stopMs / 1000.0f * sampleRate;

    std::map<int, bool> eventMap; //keeps track of events that would get rendered at the same timeline position

    for (auto info : fileReader->getActiveEventInfo())
    {
        for (int i = 0; i < info.sampleNumbers.size(); i++)
        {
            int64 sampleNumber = info.sampleNumbers[i]; //TODO: Update EventInfo object name timestamps -> sampleNumbers
            int16 state = info.channelStates[i];

            if (state && sampleNumber >= startSample && sampleNumber <= stopSample)
            {
                float timelinePos = (sampleNumber - startSample) / float (totalSamples) * getWidth();

                //if timelinePos is already in eventMap, skip overlaying a new event to avoid bogging down the GUI while scrubbing
                if (eventMap.find (timelinePos) != eventMap.end())
                    continue;
                eventMap[timelinePos] = true;
                Colour c = eventChannelColours[info.channels[i] + 1];
                g.setColour (c);

                g.setOpacity (1.0f);
                g.fillRoundedRectangle (timelinePos, 0, 1, this->getHeight() - tickHeight, 0.2);
            }
        }
    }

    /* Draw the MAX_ZOOM_DURATION_IN_SECONDS interval */
    g.setColour (findColour (ThemeColours::componentParentBackground));
    g.setOpacity (0.8f);

    if (intervalStartPosition < 0)
        return;

    setIntervalPosition (intervalStartPosition);

    // Draw the scrubber interval bounds
    g.fillRoundedRectangle (intervalStartPosition, 0, 2, this->getHeight(), 2);
    g.fillRoundedRectangle (intervalStartPosition + intervalWidth, 0, 2, this->getHeight(), 2);

    // Draw the scrubber interval as a highlight
    g.setColour (findColour (ThemeColours::menuHighlightBackground));
    g.setOpacity (0.3);
    g.fillRect (intervalStartPosition + 1, 0, intervalWidth - 2, this->getHeight()-2);

    /* Draw the current playback position */
    g.setColour (findColour (ThemeColours::defaultText));
    float timelinePos = (float) (fileReader->getPlayheadPosition() - startSample) / totalSamples * getWidth();
    g.setOpacity (1.0f);
    g.fillRoundedRectangle (timelinePos, 0, 1, this->getHeight(), 0.2);

    // Update all time labels
    fileReader->getScrubberInterface()->updateTimeLabels();
}

void FullTimeline::setIntervalPosition (int pos)
{
    intervalStartPosition = pos;

    float sampleRate = fileReader->getCurrentSampleRate();
    int64 totalSamples = (stopMs - startMs) / 1000.0f * sampleRate;
    float totalTimeInSeconds = float (totalSamples) / sampleRate;

    if (totalTimeInSeconds >= MAX_ZOOM_DURATION_IN_SECONDS)
        intervalWidth = MAX_ZOOM_DURATION_IN_SECONDS / totalTimeInSeconds * float (getWidth());
    else
        intervalWidth = getWidth();

    // Prevent interval from going out of bounds
    if (intervalStartPosition + intervalWidth > getWidth())
    {
        intervalStartPosition = getWidth() - intervalWidth;
        fileReader->getScrubberInterface()->updatePlaybackTimes();
    }
}

double FullTimeline::getIntervalDurationInSeconds()
{
    return ((stopMs - startMs) / 1000.0f);
}

void FullTimeline::mouseDown (const MouseEvent& event)
{
    if (event.x >= intervalStartPosition && event.x <= intervalStartPosition + intervalWidth)
    {
        intervalIsSelected = true;
    }
}

void FullTimeline::mouseDrag (const MouseEvent& event)
{
    if (intervalIsSelected)
    {
        if (event.x >= intervalWidth / 2 && event.x < getWidth() - intervalWidth / 2)
            intervalStartPosition = event.x - intervalWidth / 2;
    }

    repaint();
    fileReader->getScrubberInterface()->updateTimeLabels();
    fileReader->getEditor()->repaint();
}

void FullTimeline::mouseUp (const MouseEvent& event)
{
    if (intervalIsSelected)
    {
        int64 currentSample = float(getStartInterval()) / float(getWidth()) * fileReader->getCurrentNumTotalSamples() + fileReader->getPlaybackStart();
        fileReader->setCurrentSample (currentSample);
    }
    intervalIsSelected = false;
}

void ZoomTimeline::paint (Graphics& g)
{
    /* Draw timeline background */
    int tickHeight = 4;
    int borderThickness = 1;

    g.setColour (findColour (ThemeColours::componentParentBackground));

    int numTicks = 7;

    for (int i = 0; i < numTicks; i++)
    {
        float dX = float (i) / numTicks * 420;

        Path tick;
        tick.startNewSubPath (dX, 0);
        tick.lineTo (dX, tickHeight);
        g.strokePath (tick, PathStrokeType (1.0));
    }

    g.fillRect (0, tickHeight, this->getWidth(), this->getHeight() - tickHeight);
    g.setColour (findColour (ThemeColours::widgetBackground));
    g.fillRect (borderThickness, tickHeight + borderThickness, this->getWidth() - 2 * borderThickness, this->getHeight() - 2 * borderThickness - tickHeight);

    float sampleRate = fileReader->getCurrentSampleRate();
    int64 totalSamples = (stopMs - startMs) / 1000.0f * sampleRate;
    int64 intervalSamples = totalSamples > MAX_ZOOM_DURATION_IN_SECONDS * sampleRate ? MAX_ZOOM_DURATION_IN_SECONDS * sampleRate : totalSamples;

    int intervalStartPos = fileReader->getScrubberInterface()->getFullTimelineStartPosition();

    int64 offset = float (intervalStartPos) / float (getWidth()) * totalSamples;

    int64 startSampleNumber = float (startMs) / 1000.0f * sampleRate + offset;
    int64 stopSampleNumber = startSampleNumber + intervalSamples;

    for (auto info : fileReader->getActiveEventInfo())
    {
        for (int i = 0; i < info.sampleNumbers.size(); i++)
        {
            int64 sampleNumber = info.sampleNumbers[i];
            int16 state = info.channelStates[i];

            if (state && sampleNumber >= startSampleNumber && sampleNumber <= stopSampleNumber)
            {
                float timelinePos = (sampleNumber - startSampleNumber) / float (intervalSamples) * getWidth();
                Colour c = eventChannelColours[info.channels[i] + 1];
                g.setColour (c);
                g.setOpacity (1.0f);
                g.fillRect (int (timelinePos), tickHeight, 1, this->getHeight() - tickHeight);
            }
        }
    }

    /* Draw the current playback position */
    g.setColour (findColour (ThemeColours::defaultText));
    float timelinePos = (float) (fileReader->getPlayheadPosition() - startSampleNumber) / (stopSampleNumber - startSampleNumber) * getWidth();
    if (0 < timelinePos < sliderPosition + sliderWidth)
    {
        g.setOpacity (1.0f);
        g.fillRoundedRectangle (timelinePos, 0, 1, this->getHeight(), 0.2);
    }

    /* Draw the scrubber interval */
    g.setColour (findColour (ThemeColours::componentParentBackground));
    g.fillRoundedRectangle (sliderPosition, 0, sliderWidth, this->getHeight(), 2);
    g.setColour (findColour (ThemeColours::componentBackground));
    g.setOpacity (0.8f);
    g.fillRoundedRectangle (sliderPosition + 1, 1, sliderWidth - 2, this->getHeight() - 2, 2);
}

void ZoomTimeline::mouseDown (const MouseEvent& event)
{
    if (event.x > sliderPosition && event.x < sliderPosition + sliderWidth)
    {
        sliderIsSelected = true;
    }
}

void ZoomTimeline::mouseDrag (const MouseEvent& event)
{
    if (sliderIsSelected)
    {
        sliderPosition = event.x - sliderWidth / 2;
    }
    /*
    else if (rightSliderIsSelected)
    {
        if (event.x > leftSliderPosition + 1.5 * sliderWidth && event.x < getWidth() - sliderWidth / 2)
            rightSliderPosition = event.x - sliderWidth / 2;
    }
    else if (playbackRegionIsSelected)
    {
        if (leftSliderPosition >= 0 && rightSliderPosition <= getWidth() - sliderWidth)
        {
            if (event.x >= regionWidth / 2 + sliderWidth && event.x <= getWidth() - regionWidth / 2)
            {
                leftSliderPosition = event.x - regionWidth / 2 - sliderWidth;
                rightSliderPosition = event.x + regionWidth / 2 - sliderWidth;
            }
        }
    }
    */

    // Prevent slider going out of timeline bounds
    if (sliderPosition < 0)
        sliderPosition = 0;

    if (sliderPosition > getWidth() - sliderWidth)
        sliderPosition = getWidth() - sliderWidth;

    repaint();
}

void ZoomTimeline::mouseUp (const MouseEvent& event)
{
    if (sliderIsSelected)
    {
        fileReader->getScrubberInterface()->setCurrentSample (sliderPosition);
    }
    sliderIsSelected = false;
}

ScrubberInterface::ScrubberInterface (FileReader* fileReader_)
{
    fileReader = fileReader_;

    int scrubInterfaceWidth = 420;

    zoomStartTimeLabel = std::make_unique<Label> ("ZoomStartTime", "");
    zoomStartTimeLabel->setBounds (0, 30, 100, 10);
    zoomStartTimeLabel->setTooltip ("Start time of the zoom timeline");
    addAndMakeVisible (zoomStartTimeLabel.get());

    zoomMiddleTimeLabel = std::make_unique<Label> ("ZoomMidTime", "");
    zoomMiddleTimeLabel->setBounds (0.39 * scrubInterfaceWidth, 30, 100, 10);
    zoomMiddleTimeLabel->setTooltip ("Current playhead position");
    addAndMakeVisible (zoomMiddleTimeLabel.get());

    //Compute zoom end time based on start/stop time from fileReader
    int64 startMs = fileReader->getPlaybackStart() / 1000.0f;
    int64 stopMs = fileReader->getPlaybackStop() / 1000.0f;
    int64 durationMs = stopMs - startMs;
    int64 intervalSamples = durationMs > 1000*MAX_ZOOM_DURATION_IN_SECONDS ? 1000*MAX_ZOOM_DURATION_IN_SECONDS : durationMs;
    int64 endMs = startMs + intervalSamples;

    TimeParameter::TimeValue duration = TimeParameter::TimeValue (durationMs);
    TimeParameter::TimeValue endTime = TimeParameter::TimeValue (endMs);

    zoomEndTimeLabel = std::make_unique<Label> ("ZoomEndTime", duration.toString());
    zoomEndTimeLabel->setBounds (0.75 * scrubInterfaceWidth, 30, 100, 10);
    zoomEndTimeLabel->setTooltip ("End time of the zoom timeline");
    addAndMakeVisible (zoomEndTimeLabel.get());

    fullStartTimeLabel = std::make_unique<Label> ("FullStartTime", "");
    fullStartTimeLabel->setBounds (0, 100, 100, 10);
    fullStartTimeLabel->setTooltip ("Start time of the recording");
    addAndMakeVisible (fullStartTimeLabel.get());

    minStartTimeLabel = std::make_unique<Label> ("MinStartTime", "00:00:00.000");
    minStartTimeLabel->setBounds (0, 115, 100, 10);
    minStartTimeLabel->setTooltip ("Minimum start time of the recording");
    minStartTimeLabel->setAlpha (0.5f);
    addAndMakeVisible (minStartTimeLabel.get());

    fullMiddleTimeLabel = std::make_unique<Label> ("FullMidTime", "");
    fullMiddleTimeLabel->setBounds (0.39 * scrubInterfaceWidth, 108, 100, 10);
    fullMiddleTimeLabel->setTooltip ("Current playback position");
    fullMiddleTimeLabel->setAlpha (0.5f);
    addAndMakeVisible (fullMiddleTimeLabel.get());

    fullEndTimeLabel = std::make_unique<Label> ("FullEndTime", "");
    fullEndTimeLabel->setBounds (0.75 * scrubInterfaceWidth, 100, 100, 10);
    fullEndTimeLabel->setTooltip ("End time of the recording");
    addAndMakeVisible (fullEndTimeLabel.get());

    maxEndTimeLabel = std::make_unique<Label> ("MaxEndTime", "00:00:00.000");
    maxEndTimeLabel->setBounds (0.75 * scrubInterfaceWidth, 115, 100, 10);
    maxEndTimeLabel->setTooltip ("Maximum end time of the recording");
    maxEndTimeLabel->setAlpha (0.5f);
    addAndMakeVisible (maxEndTimeLabel.get());

    int padding = 30;
    zoomTimeline = std::make_unique<ZoomTimeline> (fileReader);
    zoomTimeline->setBounds (padding, 46, scrubInterfaceWidth - 2 * padding, 20);
    addAndMakeVisible (zoomTimeline.get());

    fullTimeline = std::make_unique<FullTimeline> (fileReader);
    fullTimeline->setBounds (padding, 76, scrubInterfaceWidth - 2 * padding, 20);
    addAndMakeVisible (fullTimeline.get());

    setVisible (false);
}

void ScrubberInterface::setCurrentSample (int zoomTimelinePos)
{
    // Compute the new current sample number based on the full timeline and zoom timeline slider positions
    int64 totalSamples = fullTimeline->getIntervalDurationInSeconds() * fileReader->getCurrentSampleRate();
    TimeParameter* start = static_cast<TimeParameter*> (fileReader->getParameter ("start_time"));
    int64 newCurrentSample = start->getTimeValue()->getTimeInMilliseconds() / 1000.0f * fileReader->getCurrentSampleRate();
    newCurrentSample += float (getFullTimelineStartPosition()) / fullTimeline->getWidth() * totalSamples;
    float totalTimeInSeconds = float (totalSamples) / fileReader->getCurrentSampleRate();
    float intervalWidth = totalTimeInSeconds >= MAX_ZOOM_DURATION_IN_SECONDS ? MAX_ZOOM_DURATION_IN_SECONDS : totalTimeInSeconds;
    newCurrentSample += float (zoomTimelinePos) / zoomTimeline->getWidth() * intervalWidth * fileReader->getCurrentSampleRate();
    fileReader->setCurrentSample (newCurrentSample);
}

void ScrubberInterface::updatePlaybackTimes()
{
    int64 totalSamples = fullTimeline->getIntervalDurationInSeconds() * fileReader->getCurrentSampleRate();

    TimeParameter* start = static_cast<TimeParameter*> (fileReader->getParameter ("start_time"));

    int64 newStartSample = start->getTimeValue()->getTimeInMilliseconds() / 1000.0f * fileReader->getCurrentSampleRate();
    newStartSample += float (getFullTimelineStartPosition()) / fullTimeline->getWidth() * totalSamples;
    float totalTimeInSeconds = float (totalSamples) / fileReader->getCurrentSampleRate();
    float intervalWidth = totalTimeInSeconds >= MAX_ZOOM_DURATION_IN_SECONDS ? MAX_ZOOM_DURATION_IN_SECONDS : totalTimeInSeconds;
    newStartSample += float (getZoomTimelineStartPosition()) / zoomTimeline->getWidth() * intervalWidth;
    fileReader->setPlaybackStart (newStartSample);
}

void ScrubberInterface::paintOverChildren (Graphics& g)
{
    int leftRay = fullTimeline->getStartInterval();
    int rightRay = leftRay + fullTimeline->getIntervalWidth();

    g.drawLine (zoomTimeline->getX(), zoomTimeline->getY() + zoomTimeline->getHeight(), zoomTimeline->getX() + leftRay, fullTimeline->getY());
    g.drawLine (zoomTimeline->getX() + zoomTimeline->getWidth(), zoomTimeline->getY() + zoomTimeline->getHeight(), zoomTimeline->getX() + rightRay, fullTimeline->getY());
}

void ScrubberInterface::buttonClicked (Button* button)
{
    //playbackButton->setState (! playbackButton->getState()); //deprecated
    updatePlaybackTimes();
    fileReader->togglePlayback();
}

int ScrubberInterface::getFullTimelineStartPosition()
{
    return fullTimeline->getStartInterval();
}

int ScrubberInterface::getZoomTimelineStartPosition()
{
    return zoomTimeline->getStartInterval();
}

void ScrubberInterface::update()
{
    TimeParameter* start = static_cast<TimeParameter*> (fileReader->getParameter ("start_time"));
    fullStartTimeLabel->setText (start->getTimeValue()->toString(), juce::sendNotificationAsync);
    int minStartTime = start->getTimeValue()->getMinTimeInMilliseconds();
    minStartTimeLabel->setText (TimeParameter::TimeValue (minStartTime).toString(), juce::sendNotificationAsync);

    TimeParameter* stop = static_cast<TimeParameter*> (fileReader->getParameter ("end_time"));
    fullEndTimeLabel->setText (stop->getTimeValue()->toString(), juce::sendNotificationAsync);
    int maxEndTime = stop->getTimeValue()->getMaxTimeInMilliseconds();
    maxEndTimeLabel->setText (TimeParameter::TimeValue (maxEndTime).toString(), juce::sendNotificationAsync);

    int startMs = start->getTimeValue()->getTimeInMilliseconds();
    int stopMs = stop->getTimeValue()->getTimeInMilliseconds();
    int duration = stopMs - startMs;

    fullTimeline->setStartStopTimes (startMs, stopMs);
    zoomTimeline->setStartStopTimes (startMs, stopMs);

    FileReaderEditor* e = static_cast<FileReaderEditor*> (fileReader->getEditor());


    e->repaint();
}

void ScrubberInterface::updateTimeLabels()
{
    int start = ((TimeParameter*) fileReader->getParameter ("start_time"))->getTimeValue()->getTimeInMilliseconds();
    int stop = ((TimeParameter*) fileReader->getParameter ("end_time"))->getTimeValue()->getTimeInMilliseconds();

    int duration = (stop - start);

    int startPos = fullTimeline->getStartInterval();
    float frac = float (startPos) / float (fullTimeline->getWidth());

    // Update zoom start time label from fullTimeline slider position
    int pos = fullTimeline->getStartInterval();
    float startTime = float(pos)/float(fullTimeline->getWidth()) * duration;
    TimeParameter::TimeValue time = TimeParameter::TimeValue (start + startTime);
    zoomStartTimeLabel->setText (time.toString(), juce::sendNotificationAsync);
    int64 zoomStartTime = time.getTimeInMilliseconds();

    // Update zoom end time label
    int64 zoomEndTime;
    if (duration >= 1000*MAX_ZOOM_DURATION_IN_SECONDS)
    {
        TimeParameter::TimeValue time = TimeParameter::TimeValue (start + frac * duration + 1000*MAX_ZOOM_DURATION_IN_SECONDS);
        zoomEndTime = time.getTimeInMilliseconds();
        zoomEndTimeLabel->setText (time.toString(), juce::sendNotificationAsync);
    }
    else
    {
        TimeParameter::TimeValue time = TimeParameter::TimeValue (start + frac * duration + (stop - start));
        zoomEndTime = time.getTimeInMilliseconds();
        zoomEndTimeLabel->setText (time.toString(), juce::sendNotificationAsync);
    }

    // Update zoom middle time label
    int currentPos = ((ZoomTimeline*)zoomTimeline.get())->getSliderPosition();
    int64 zoomMiddleTime = zoomStartTime + float(currentPos) / zoomTimeline->getWidth() * (zoomEndTime - zoomStartTime);
    TimeParameter::TimeValue middleTime = TimeParameter::TimeValue (zoomMiddleTime);
    zoomMiddleTimeLabel->setText (middleTime.toString(), juce::sendNotificationAsync);

    // Get current playhead time from currentSample
    float sampleRate = fileReader->getCurrentSampleRate();
    int64 currentSample = fileReader->getPlayheadPosition();
    float currentTime = currentSample / sampleRate * 1000.0f;
    TimeParameter::TimeValue currentTimeValue = TimeParameter::TimeValue (currentTime);
    fullMiddleTimeLabel->setText (currentTimeValue.toString(), juce::sendNotificationAsync);
}