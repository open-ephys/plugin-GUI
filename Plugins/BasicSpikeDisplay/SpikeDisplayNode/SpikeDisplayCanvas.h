/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 /Users/franman/Documents/Programming/GitHub/plugin-GUI/Plugins/BasicSpikeDisplay/SpikeDisplayNode/SpikeDisplayCanvas.h

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SPIKEDISPLAYCANVAS_H_
#define SPIKEDISPLAYCANVAS_H_

#include <VisualizerWindowHeaders.h>

#include "SpikeDisplay.h"

#include <vector>

class SpikePlot;
class SpikeDisplayNode;

class SpikeDisplayCache
{
public:
    SpikeDisplayCache () {}
    virtual ~SpikeDisplayCache() {}

    void setMonitor(std::string key, bool isMonitored) {
        monitors[key] = isMonitored;
    };

    bool isMonitored(std::string key) {
        return monitors[key];
    };

    void setRange(std::string key, int channelIdx, double range) {
        ranges[key][channelIdx] = range;
    };

    double getRange(std::string key, int channelIdx) {
        return ranges[key][channelIdx];
    };

    void setThreshold(std::string key,int channelIdx, double thresh) {
        thresholds[key][channelIdx] = thresh;
    };

    double getThreshold(std::string key, int channelIdx) {
        return thresholds[key][channelIdx];
    };

    bool hasCachedDisplaySettings(std::string cacheKey)
    {
        /*
        LOGDD("SpikeDisplayCache keys:");
        std::vector<std::string> keys = extract_keys(ranges);
        std::vector<std::map<int,double>> vals = extract_values(ranges);
        for (int i = 0; i < keys.size(); i++)
        {
            std::vector<int> channels = extract_keys(vals[i]);
            std::vector<double> ranges = extract_values(vals[i]);
            for (int j = 0; j < channels.size(); j++)
                LOGDD("Key: ", keys[i], " Channel: ", channels[j], " Range: ", ranges[j]);
        }
        */
        return thresholds.count(cacheKey) > 0;
    };

    std::string findSimilarKey(std::string key, int streamIndex)
    {
        std::vector<std::string> keys = extract_keys(ranges);

        unsigned sourcePos = 0;
        unsigned streamPos = key.find_first_of("|");
        unsigned namePos = key.find_last_of("|");

        // First check for a source ID change (match only stream + electrode name)
        for (int i = 0; i < keys.size(); i++)
        {
            std::string partToMatch = key.substr(streamPos, key.length() - streamPos);
            std::string possibleMatch = keys[i].substr(streamPos, keys[i].length() - streamPos);
            if (partToMatch.compare(possibleMatch) == 0)
                return keys[i];
        }

        // Next check for a stream name change (match only node + electrode name)
        std::vector<std::string> matches;
        for (int i = 0; i < keys.size(); i++)
        {
            int namePos2 = keys[i].find_last_of("|");
            std::string partToMatch = key.substr(sourcePos, streamPos - sourcePos) + key.substr(namePos, key.length() - namePos);
            std::string possibleMatch = keys[i].substr(sourcePos, streamPos - sourcePos) + keys[i].substr(namePos2, keys[i].length() - namePos2);
            if (partToMatch.compare(possibleMatch) == 0)
                matches.push_back(keys[i]);
        }

        // Check if multiple matches, if so, default to stream index
        if (matches.size() == 1)
            return matches[0];
        else if (matches.size() > streamIndex)
            return matches[streamIndex];

        // No match found
        return "";
    }

private:

    std::map<std::string, std::map<int, double>> ranges;
    std::map<std::string, std::map<int, double>> thresholds;
    std::map<std::string, bool> monitors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDisplayCache);
};

/**
    Allows spike plot thresholds to be adjusted synchronously
*/
class SpikeThresholdCoordinator
{
public:

    /** Constructor*/
    SpikeThresholdCoordinator();

    /** Destructor*/
    ~SpikeThresholdCoordinator();

    /** Registers a plot to interact with this coordinator*/
    void registerSpikePlot(SpikePlot* sp);

    /** De-registers a plot to interact with this coordinators*/
    void deregisterSpikePlot(SpikePlot* sp);

    /** Sets the lock threshold state*/
    void setLockThresholds(bool en);

    /** Returns the lock threshold state*/
    bool getLockThresholds();

    /** Sets the thresholds of all registered plots*/
    void thresholdChanged(float displayThreshold, float range);

private:

    bool lockThresholds;
    Array<SpikePlot*> registeredPlots;

    WeakReference<SpikeThresholdCoordinator>::Master masterReference;
    friend class WeakReference<SpikeThresholdCoordinator>;

};

/**

  Displays spike waveforms and projections.

  @see SpikeDisplayNode, SpikeDisplayEditor, Visualizer

*/

class SpikeDisplayCanvas : public Visualizer, 
                           public Button::Listener

{
public:

    /** Constructor */
    SpikeDisplayCanvas(SpikeDisplayNode* n);

    /** Destructor */
    ~SpikeDisplayCanvas() { }

    /** Render black background */
    void paint(Graphics& g);

    /** Called instead of "repaint" to avoid redrawing underlying components.*/
    void refresh();

    /** Called when the component's tab becomes visible again*/
    void refreshState();

    /** Creates spike displays for incoming spike channels*/
    void update();

    /** Aligns components*/
    void resized();

    /** Respond to clear / lock thresholds / invert spikes buttons*/
    void buttonClicked(Button* button);

    /** Clears audio monitor selection for all sub-plots*/
    void resetAudioMonitorState();

    /** Sets the scaling facotr for the sub-plots*/
    void setPlotScaleFactor(float scale);

    /** Saves display parameters */
    void saveCustomParametersToXml(XmlElement* xml);

    /** Loads display parameters */
    void loadCustomParametersFromXml(XmlElement* xml);

    /** Apply cached settings */
    void applyCachedDisplaySettings(int plotIdx, std::string cacheKey);

    /** Pointer to the underlying SpikeDisplayNode*/
    SpikeDisplayNode* processor;

    /** Manages connections from SpikeChannels to SpikePlots */
    std::unique_ptr<SpikeDisplayCache> cache;

private:

    std::unique_ptr<SpikeDisplay> spikeDisplay;
    std::unique_ptr<Viewport> viewport;

    bool newSpike;

    int scrollBarThickness;

    std::unique_ptr<UtilityButton> clearButton;
    std::unique_ptr<SpikeThresholdCoordinator> thresholdCoordinator;
    std::unique_ptr<UtilityButton> lockThresholdsButton;
    std::unique_ptr<UtilityButton> invertSpikesButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDisplayCanvas);

};


#endif  // SPIKEDISPLAYCANVAS_H_
