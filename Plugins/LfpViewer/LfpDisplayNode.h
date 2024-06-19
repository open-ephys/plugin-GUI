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

#ifndef __LFPDISPLAYNODE_H_Alpha__
#define __LFPDISPLAYNODE_H_Alpha__

#include "DisplayBuffer.h"
#include "LfpDisplayCanvas.h"
#include "LfpDisplayEditor.h"
#include <ProcessorHeaders.h>

#include <map>

class DataViewport;

namespace LfpViewer
{

/**

  Holds data in a displayBuffer to be used by the LfpDisplayCanvas
  for rendering continuous data streams.

  @see GenericProcessor, LfpDisplayEditor, LfpDisplayCanvas

*/
class TESTABLE LfpDisplayNode : public GenericProcessor

{
public:
    /** Constructor */
    LfpDisplayNode();

    /** Destructor*/
    ~LfpDisplayNode() {}

    /** Creates the LfpDisplayEditor*/
    AudioProcessorEditor* createEditor() override;

    /** Pushes incoming data into a drawing buffer*/
    void process (AudioBuffer<float>& buffer) override;

    /** Used to set display trigger channels*/
    void setParameter (int parameterIndex, float newValue) override;

    /** Creates buffers for incoming data streams*/
    void updateSettings() override;

    /** Informs editor whether or not the signal chain is loading, to prevent unnecessary redraws*/
    void initialize (bool signalChainIsLoading) override;

    /** Starts display animation */
    bool startAcquisition() override;

    /** Stops display animations*/
    bool stopAcquisition() override;

    /** Highlights recorded channels */
    void startRecording() override;

    /** Un-highlights recorded channels*/
    void stopRecording() override;

    /** Used for TTL event overlay*/
    void handleTTLEvent (TTLEventPtr event) override;

    /** Returns an array of pointers to the availble displayBuffers*/
    Array<DisplayBuffer*> getDisplayBuffers();

    /** Map between data stream keys and display buffer pointers*/
    std::map<uint16, DisplayBuffer*> displayBufferMap;

    /** Sets an array of pointers to the 3 available split displays*/
    void setSplitDisplays (Array<LfpDisplaySplitter*>);

    /** Returns the latest sample number that triggered a given split display*/
    int64 getLatestTriggerTime (int splitId) const;

    /** Acknowledges receipt of a trigger for a given split display*/
    void acknowledgeTrigger (int splitId);

    /** Handles messages from other processors during acquisition*/
    void handleBroadcastMessage (const String& msg, const int64 messageTimeMillis) override;

    bool getHeadlessMode() const
    {
        return headlessMode;
    }

private:
    /** Initializes trigger channels within a process block*/
    void initializeEventChannels();

    /** Called after all events have been received within a process block*/
    void finalizeEventChannels();

    OwnedArray<DisplayBuffer> displayBuffers;

    Array<LfpDisplaySplitter*> splitDisplays;

    Array<int> triggerChannels;
    Array<int64> latestTrigger; // overall timestamp
    Array<int> latestCurrentTrigger; // within current input buffer

    static uint16 getEventSourceId (const EventChannel* event);
    static uint16 getChannelSourceId (const ChannelInfoObject* chan);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfpDisplayNode);
};
}; // namespace LfpViewer

#endif // __LFPDISPLAYNODE_H_Alpha__
