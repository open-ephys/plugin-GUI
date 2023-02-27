/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

SpikeThresholdCoordinator::SpikeThresholdCoordinator() :
    lockThresholds(false)
{

}

SpikeThresholdCoordinator::~SpikeThresholdCoordinator()
{
    masterReference.clear();
}

void SpikeThresholdCoordinator::registerSpikePlot(SpikePlot* sp)
{
    registeredPlots.addIfNotAlreadyThere(sp);
}

void SpikeThresholdCoordinator::deregisterSpikePlot(SpikePlot* sp)
{
    registeredPlots.removeAllInstancesOf(sp);
}

void SpikeThresholdCoordinator::setLockThresholds(bool en)
{
    lockThresholds = en;
}

bool SpikeThresholdCoordinator::getLockThresholds()
{
    return lockThresholds;
}

void SpikeThresholdCoordinator::thresholdChanged(float displayThreshold, float range)
{
    if (lockThresholds)
    {
        for (int i = 0; i < registeredPlots.size(); i++)
        {
            registeredPlots[i]->setAllThresholds(displayThreshold, range);
        }
    }
}


SpikeDisplayCanvas::SpikeDisplayCanvas(SpikeDisplayNode* processor_) :
    processor(processor_), 
    newSpike(false)
{

    refreshRate = 10; // Hz

    viewport = std::make_unique<Viewport>();
    spikeDisplay = std::make_unique<SpikeDisplay>(this, viewport.get());

    thresholdCoordinator = std::make_unique < SpikeThresholdCoordinator>();
    spikeDisplay->registerThresholdCoordinator(thresholdCoordinator.get());

    viewport->setViewedComponent(spikeDisplay.get(), false);
    viewport->setScrollBarsShown(true, false);

    scrollBarThickness = viewport->getScrollBarThickness();

    clearButton = std::make_unique <UtilityButton>("Clear Plots", Font("Small Text", 13, Font::plain));
    clearButton->setRadius(3.0f);
    clearButton->addListener(this);
    addAndMakeVisible(clearButton.get());

    lockThresholdsButton = std::make_unique<UtilityButton>("Lock Thresholds", Font("Small Text", 13, Font::plain));
    lockThresholdsButton->setRadius(3.0f);
    lockThresholdsButton->addListener(this);
    lockThresholdsButton->setClickingTogglesState(true);
    addAndMakeVisible(lockThresholdsButton.get());

    invertSpikesButton = std::make_unique <UtilityButton>("Invert Spikes", Font("Small Text", 13, Font::plain));
    invertSpikesButton->setRadius(3.0f);
    invertSpikesButton->addListener(this);
    invertSpikesButton->setClickingTogglesState(true);
    invertSpikesButton->setToggleState(false, sendNotification);
    addAndMakeVisible(invertSpikesButton.get());

    addAndMakeVisible(viewport.get());

    update();

    cache = std::make_unique<SpikeDisplayCache>();
}

void SpikeDisplayCanvas::applyCachedDisplaySettings(int plotIdx, std::string key)
{
    spikeDisplay->setMonitorStateForPlot(plotIdx, cache->isMonitored(key));

    for (int j = 0; j < processor->getNumberOfChannelsForElectrode(plotIdx); j++)
    {
        spikeDisplay->setThresholdForWaveAxis(plotIdx,j,cache->getThreshold(key,j));
        spikeDisplay->setRangeForWaveAxis(plotIdx,j,cache->getRange(key, j));
    }
}

void SpikeDisplayCanvas::update()
{

    int scrollHeight = viewport->getViewPositionY();

    int nPlots = processor->getNumElectrodes();
    
    processor->removeSpikePlots();

    spikeDisplay->removePlots();

    for (int i = 0; i < nPlots; i++)
    {
        SpikePlot* sp = spikeDisplay->addSpikePlot(processor->getNumberOfChannelsForElectrode(i), i,
                                                   processor->getNameForElectrode(i), processor->getSpikeChannel(i)->getIdentifier().toStdString());

        processor->addSpikePlotForElectrode(sp, i);

        std::string cacheKey = processor->getSpikeChannel(i)->getIdentifier().toStdString();

        if (cache)
        {
            //TODO: Should be able to call spikeChannel->getStreamIndex() here...
            int streamIdx = 0;
            for (auto& stream : processor->getDataStreams())
            {
                if (stream->getStreamId() == processor->getSpikeChannel(i)->getStreamId())
                    break;
                streamIdx++;
            }

            if (cache->hasCachedDisplaySettings(cacheKey))
            {
                LOGD("SpikeDisplayCanvas::update: found exact key for ", cacheKey);
                applyCachedDisplaySettings(i, cacheKey);
            }
            else if (cache->findSimilarKey(cacheKey, streamIdx).size() > 0)
            {
                LOGD("SpikeDisplayCanvas::update: found similar key for ", cacheKey);
                applyCachedDisplaySettings(i, cache->findSimilarKey(cacheKey, streamIdx));
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
    viewport->setBounds(0, 0, getWidth(), getHeight()-30); // leave space at bottom for buttons

    spikeDisplay->setBounds(0,0,getWidth()-scrollBarThickness, spikeDisplay->getTotalHeight());

    clearButton->setBounds(10, getHeight()-25, 130, 20);

    lockThresholdsButton->setBounds(10+130+10, getHeight()-25, 130, 20);

    invertSpikesButton->setBounds(10+130+10+130+10, getHeight()-25, 130, 20);

}

void SpikeDisplayCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::black);
}

void SpikeDisplayCanvas::refresh()
{
    spikeDisplay->refresh();
}


void SpikeDisplayCanvas::buttonClicked(Button* button)
{

    if (button == clearButton.get())
    {
        spikeDisplay->clear();
    }
    else if (button == lockThresholdsButton.get())
    {
        thresholdCoordinator->setLockThresholds(button->getToggleState());
    }
    else if (button == invertSpikesButton.get())
    {
        spikeDisplay->invertSpikes(button->getToggleState());
    }
}

void SpikeDisplayCanvas::resetAudioMonitorState()
{
    spikeDisplay->resetAudioMonitorState();
}


void SpikeDisplayCanvas::setPlotScaleFactor(float scale)
{
    spikeDisplay->setPlotScaleFactor(scale);
}

void SpikeDisplayCanvas::saveCustomParametersToXml(XmlElement* xml)
{

    XmlElement* xmlNode = xml->createNewChildElement("SPIKEDISPLAY");

    xmlNode->setAttribute("LockThresholds", lockThresholdsButton->getToggleState());
    xmlNode->setAttribute("InvertSpikes", invertSpikesButton->getToggleState());

    int spikePlotIdx = -1;

    for (int i = 0; i < processor->getTotalSpikeChannels(); i++)
    {
        if (!processor->getSpikeChannel(i)->isValid())
            continue;

        spikePlotIdx++;

        const SpikeChannel* spikeChannel = processor->getSpikeChannel(i);

        const uint16 streamId = spikeChannel->getStreamId();

        XmlElement* plotNode = xmlNode->createNewChildElement("PLOT");

        plotNode->setAttribute("stream_source", processor->getDataStream(streamId)->getSourceNodeId());
        plotNode->setAttribute("stream_name", processor->getDataStream(streamId)->getName());
        plotNode->setAttribute("source_node", spikeChannel->getSourceNodeId());
        plotNode->setAttribute("name", spikeChannel->getName());

        plotNode->setAttribute("isMonitored", spikeDisplay->getMonitorStateForPlot(i));

        for (int j = 0; j < spikeDisplay->getNumChannelsForPlot(i); j++)
        {
            XmlElement* axisNode = plotNode->createNewChildElement("AXIS");
            axisNode->setAttribute("thresh",spikeDisplay->getThresholdForWaveAxis(i,j));
            axisNode->setAttribute("range",spikeDisplay->getRangeForWaveAxis(i,j));
        }
    }

    xmlNode->setAttribute("NumPlots", spikePlotIdx+1);

}

void SpikeDisplayCanvas::loadCustomParametersFromXml(XmlElement* xml)
{

    for (auto* xmlNode : xml->getChildIterator())
    {
        if (xmlNode->hasTagName("SPIKEDISPLAY"))
        {

            if (xmlNode->getIntAttribute("NumPlots", 0) != processor->getTotalSpikeChannels())
            {
                //SpikeDisplayNode has not loaded all spike channels from all incoming branches yet.
                //Wait until the processor has loaded all channels before loading the saved settings.
                return;
            }

            spikeDisplay->invertSpikes(xmlNode->getBoolAttribute("InvertSpikes"));
            invertSpikesButton->setToggleState(xmlNode->getBoolAttribute("InvertSpikes"), dontSendNotification);
            lockThresholdsButton->setToggleState(xmlNode->getBoolAttribute("LockThresholds"), sendNotification);

            int plotIndex = -1;

            for (auto* plotNode : xmlNode->getChildIterator())
            {
                if (plotNode->hasTagName("PLOT"))
                {
                    plotIndex++;

                    std::string stream_source = plotNode->getStringAttribute("stream_source").toStdString();
                    std::string stream_name = plotNode->getStringAttribute("stream_name").toStdString();
                    std::string source = plotNode->getStringAttribute("source_node").toStdString();
                    std::string name = plotNode->getStringAttribute("name").toStdString();

                    std::string cacheKey = stream_source + "|" + stream_name + "|" + source + "|" + name;

                    cache->setMonitor(cacheKey, plotNode->getBoolAttribute("isMonitored"));

                    int channelIndex = -1;

                    for (auto* channelNode : plotNode->getChildIterator())
                    {
                        if (channelNode->hasTagName("AXIS"))
                        {
                            channelIndex++;
                            cache->setThreshold(cacheKey, channelIndex, channelNode->getDoubleAttribute("thresh"));
                            cache->setRange(cacheKey, channelIndex, channelNode->getDoubleAttribute("range"));
                        }
                    }
                }
            }
        }
    }

}
