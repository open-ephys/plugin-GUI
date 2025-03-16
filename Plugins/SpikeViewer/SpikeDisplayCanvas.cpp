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

#include "SpikeDisplayCanvas.h"

#include "SpikeDisplayNode.h"
#include "SpikePlots.h"

SpikeThresholdCoordinator::SpikeThresholdCoordinator() : lockThresholds (false)
{
}

SpikeThresholdCoordinator::~SpikeThresholdCoordinator()
{
    masterReference.clear();
}

void SpikeThresholdCoordinator::registerSpikePlot (SpikePlot* sp)
{
    registeredPlots.addIfNotAlreadyThere (sp);
}

void SpikeThresholdCoordinator::deregisterSpikePlot (SpikePlot* sp)
{
    registeredPlots.removeAllInstancesOf (sp);
}

void SpikeThresholdCoordinator::setLockThresholds (bool en)
{
    lockThresholds = en;
}

bool SpikeThresholdCoordinator::getLockThresholds()
{
    return lockThresholds;
}

void SpikeThresholdCoordinator::thresholdChanged (float displayThreshold, float range)
{
    if (lockThresholds)
    {
        for (int i = 0; i < registeredPlots.size(); i++)
        {
            registeredPlots[i]->setAllThresholds (displayThreshold, range);
        }
    }
}

SpikeDisplayCanvas::SpikeDisplayCanvas (SpikeDisplayNode* processor_) : Visualizer (processor_),
                                                                        processor (processor_),
                                                                        newSpike (false)
{
    refreshRate = 10; // Hz

    FontOptions labelFont ("Inter", "Regular", 16.0f);

    // SPIKE DISPLAY
    viewport = std::make_unique<Viewport>();
    spikeDisplay = std::make_unique<SpikeDisplay> (this, viewport.get());

    scrollBarThickness = 15;
    viewport->setViewedComponent (spikeDisplay.get(), false);
    viewport->setScrollBarsShown (true, false);
    viewport->setScrollBarThickness (scrollBarThickness);
    addAndMakeVisible (viewport.get());

    // MAIN OPTIONS
    mainOptionsHolder = std::make_unique<Viewport>();
    mainOptionsHolder->setScrollBarsShown (false, true);
    mainOptionsHolder->setScrollBarThickness (10);
    addAndMakeVisible (mainOptionsHolder.get());

    mainOptions = std::make_unique<Component> ("Main options");
    mainOptionsHolder->setViewedComponent (mainOptions.get(), false);

    // Range selection
    rangeSelection = std::make_unique<ComboBox> ("Range");
    for (auto range : ranges)
        rangeSelection->addItem (String (range) + " uV", range);
    rangeSelection->setSelectedId (ranges[2], dontSendNotification);
    rangeSelection->addListener (this);
    mainOptions->addAndMakeVisible (rangeSelection.get());

    rangeSelectionLabel = std::make_unique<Label> ("RangeSelectionLabel", "Range:");
    rangeSelectionLabel->setFont (labelFont);
    mainOptions->addAndMakeVisible (rangeSelectionLabel.get());

    // History selection
    historySelection = std::make_unique<ComboBox> ("History");
    for (auto history : histories)
        historySelection->addItem (String (history), history);
    historySelection->setSelectedId (histories[1], dontSendNotification);
    historySelection->addListener (this);
    mainOptions->addAndMakeVisible (historySelection.get());

    historySelectionLabel = std::make_unique<Label> ("HistorySelectionLabel", "Spike History:");
    historySelectionLabel->setFont (labelFont);
    mainOptions->addAndMakeVisible (historySelectionLabel.get());

    // Threshold lock
    thresholdCoordinator = std::make_unique<SpikeThresholdCoordinator>();
    spikeDisplay->registerThresholdCoordinator (thresholdCoordinator.get());

    lockThresholdsButton = std::make_unique<UtilityButton> ("Lock Thresholds");
    lockThresholdsButton->setRadius (3.0f);
    lockThresholdsButton->addListener (this);
    lockThresholdsButton->setClickingTogglesState (true);
    mainOptions->addAndMakeVisible (lockThresholdsButton.get());

    // Clear button
    clearButton = std::make_unique<UtilityButton> ("Clear Plots");
    clearButton->setRadius (3.0f);
    clearButton->addListener (this);
    mainOptions->addAndMakeVisible (clearButton.get());

    // Invert spikes
    invertSpikesButton = std::make_unique<UtilityButton> ("Invert Spikes");
    invertSpikesButton->setRadius (3.0f);
    invertSpikesButton->addListener (this);
    invertSpikesButton->setClickingTogglesState (true);
    invertSpikesButton->setToggleState (false, sendNotification);
    mainOptions->addAndMakeVisible (invertSpikesButton.get());

    updateSettings();

    cache = std::make_unique<SpikeDisplayCache>();
}

void SpikeDisplayCanvas::updateSettings()
{
    int scrollHeight = viewport->getViewPositionY();

    int nPlots = processor->getNumElectrodes();

    processor->removeSpikePlots();

    spikeDisplay->removePlots();

    for (int i = 0; i < nPlots; i++)
    {
        SpikePlot* sp = spikeDisplay->addSpikePlot (processor->getNumberOfChannelsForElectrode (i), 
                                                    i,
                                                    processor->getNameForElectrode (i), 
                                                    processor->getChannelNamesForElectrode(i),
                                                    processor->getSpikeChannel (i)->getIdentifier().toStdString());

        processor->addSpikePlotForElectrode (sp, i);

        std::string cacheKey = processor->getSpikeChannel (i)->getIdentifier().toStdString();

        if (cache && cache->hasCachedDisplaySettings (cacheKey))
        {
            spikeDisplay->setMonitorStateForPlot (i, cache->isMonitored (cacheKey));

            for (int j = 0; j < processor->getNumberOfChannelsForElectrode (i); j++)
            {
                spikeDisplay->setThresholdForWaveAxis (i, j, cache->getThreshold (cacheKey, j));
                spikeDisplay->setRangeForWaveAxis (i, j, cache->getRange (cacheKey, j));
            }
        }
    }

    spikeDisplay->resized();
    spikeDisplay->refresh();
}

void SpikeDisplayCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    resized();
}

void SpikeDisplayCanvas::resized()
{
    int optionsHeight = 40;
    int border = 10;

    viewport->setBounds (0, 0, getWidth(), getHeight() - optionsHeight - 5); // leave space at bottom for buttons

    spikeDisplay->setBounds (0, 0, getWidth() - scrollBarThickness, 9999); // once to calculate the total height
    spikeDisplay->setBounds (0, 0, getWidth() - scrollBarThickness, spikeDisplay->getTotalHeight()); // again to set the height

    mainOptionsHolder->setBounds (0, getHeight() - optionsHeight, getWidth(), optionsHeight);

    rangeSelectionLabel->setBounds (10, 10, 55, 20);
    rangeSelection->setBounds (rangeSelectionLabel->getRight() + border - 10, 10, 90, 20);

    historySelectionLabel->setBounds (rangeSelection->getRight() + border, 10, 100, 20);
    historySelection->setBounds (historySelectionLabel->getRight() + border - 10, 10, 80, 20);

    clearButton->setBounds (historySelection->getRight() + border * 2, 10, 130, 20);

    lockThresholdsButton->setBounds (clearButton->getRight() + border, 10, 130, 20);

    invertSpikesButton->setBounds (lockThresholdsButton->getRight() + border, 10, 130, 20);

    mainOptions->setBounds (0, 0, invertSpikesButton->getRight(), optionsHeight);
}

void SpikeDisplayCanvas::paint (Graphics& g)
{
    g.fillAll (findColour (ThemeColours::componentParentBackground));

    // Fill the main options area
    g.setColour (findColour (ThemeColours::componentBackground));
    g.fillRect (mainOptionsHolder->getBounds());
}

void SpikeDisplayCanvas::refresh()
{
    spikeDisplay->refresh();
}

void SpikeDisplayCanvas::buttonClicked (Button* button)
{
    if (button == clearButton.get())
    {
        spikeDisplay->clear();
    }
    else if (button == lockThresholdsButton.get())
    {
        thresholdCoordinator->setLockThresholds (button->getToggleState());
    }
    else if (button == invertSpikesButton.get())
    {
        spikeDisplay->invertSpikes (button->getToggleState());
    }
}

void SpikeDisplayCanvas::comboBoxChanged (ComboBox* comboBox)
{
    if (comboBox == historySelection.get())
    {
        spikeDisplay->setHistorySize ((int) historySelection->getSelectedId());
    }
    else if (comboBox == rangeSelection.get())
    {
        spikeDisplay->setRange ((int) rangeSelection->getSelectedId());
    }
}

void SpikeDisplayCanvas::resetAudioMonitorState()
{
    spikeDisplay->resetAudioMonitorState();
}

void SpikeDisplayCanvas::setPlotScaleFactor (float scale)
{
    spikeDisplay->setPlotScaleFactor (scale);
}

void SpikeDisplayCanvas::saveCustomParametersToXml (XmlElement* xml)
{
    XmlElement* xmlNode = xml->createNewChildElement ("SPIKEDISPLAY");

    xmlNode->setAttribute ("LockThresholds", lockThresholdsButton->getToggleState());
    xmlNode->setAttribute ("InvertSpikes", invertSpikesButton->getToggleState());
    xmlNode->setAttribute ("Range", rangeSelection->getSelectedId());
    xmlNode->setAttribute ("History", historySelection->getSelectedId());

    int spikePlotIdx = -1;

    for (int i = 0; i < processor->getTotalSpikeChannels(); i++)
    {
        if (! processor->getSpikeChannel (i)->isValid())
            continue;

        spikePlotIdx++;

        XmlElement* plotNode = xmlNode->createNewChildElement ("PLOT");

        //plotNode->setAttribute("name", processor->getSpikeChannel(i)->getIdentifier());
        String identifier = processor->getSpikeChannel (i)->getIdentifier();

        StringArray tokens;
        tokens.addTokens (identifier, "|", "\"");

        plotNode->setAttribute ("stream_source", tokens[0]);
        plotNode->setAttribute ("stream_name", tokens[1]);
        plotNode->setAttribute ("spike_source", tokens[2]);
        plotNode->setAttribute ("name", tokens[3]);

        plotNode->setAttribute ("isMonitored", spikeDisplay->getMonitorStateForPlot (i));

        for (int j = 0; j < spikeDisplay->getNumChannelsForPlot (spikePlotIdx); j++)
        {
            XmlElement* axisNode = plotNode->createNewChildElement ("AXIS");
            axisNode->setAttribute ("thresh", spikeDisplay->getThresholdForWaveAxis (spikePlotIdx, j));
            axisNode->setAttribute ("range", spikeDisplay->getRangeForWaveAxis (spikePlotIdx, j));
        }
    }

    xmlNode->setAttribute ("NumPlots", spikePlotIdx + 1);
}

void SpikeDisplayCanvas::loadCustomParametersFromXml (XmlElement* xml)
{
    for (auto* xmlNode : xml->getChildIterator())
    {
        if (xmlNode->hasTagName ("SPIKEDISPLAY"))
        {
            loadSpikeDisplaySettingsFromXml (xmlNode);
        }
    }
}

void SpikeDisplayCanvas::loadSpikeDisplaySettingsFromXml (XmlElement* xmlNode)
{
    int numPlots = xmlNode->getIntAttribute ("NumPlots", 0);
    int numSpikeChannels = processor->getTotalSpikeChannels();

    if (! (numPlots && numPlots == numSpikeChannels))
    {
        //SpikeDisplayNode has not loaded all spike channels from all incoming branches yet.
        //Wait until the processor has loaded all channels before loading the saved settings.
        return;
    }

    spikeDisplay->invertSpikes (xmlNode->getBoolAttribute ("InvertSpikes"));
    invertSpikesButton->setToggleState (xmlNode->getBoolAttribute ("InvertSpikes"), dontSendNotification);
    lockThresholdsButton->setToggleState (xmlNode->getBoolAttribute ("LockThresholds"), sendNotification);
    rangeSelection->setSelectedId (xmlNode->getIntAttribute ("Range", ranges[2]), dontSendNotification);
    historySelection->setSelectedId (xmlNode->getIntAttribute ("History", histories[1]), sendNotification);

    int plotIndex = -1;

    for (auto* plotNode : xmlNode->getChildIterator())
    {
        if (plotNode->hasTagName ("PLOT"))
        {
            plotIndex++;

            std::string cacheKey = processor->getSpikeChannel (plotIndex)->getIdentifier().toStdString();

            cache->setMonitor (cacheKey, plotNode->getBoolAttribute ("isMonitored"));

            int channelIndex = -1;

            for (auto* channelNode : plotNode->getChildIterator())
            {
                if (channelNode->hasTagName ("AXIS"))
                {
                    channelIndex++;
                    cache->setThreshold (cacheKey, channelIndex, channelNode->getDoubleAttribute ("thresh"));
                    cache->setRange (cacheKey, channelIndex, channelNode->getDoubleAttribute ("range"));
                }
            }
        }
    }
}
