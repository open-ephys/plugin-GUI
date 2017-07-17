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

#ifndef CROSSING_DETECTOR_H_INCLUDED
#define CROSSING_DETECTOR_H_INCLUDED

#ifdef _WIN32
#include <Windows.h>
#endif

#include <ProcessorHeaders.h>
#include <algorithm> // max

/*
 * The crossing detector plugin is designed to read in one continuous channel c, and generate events on one events channel
 * when c crosses a certain value. There are various parameters to tweak this basic functionality, including:
 *  - whether to listen for crosses with a positive or negative slope, or either
 *  - how strictly to filter transient level changes, by adjusting the required number and percent of past and future samples to be above/below the threshold
 *  - the duration of the generated event
 *  - the minimum time to wait between events ("timeout")
 *
 * All ontinuous signals pass through unchanged, so multiple CrossingDetectors can be
 * chained together in order to operate on more than one channel.
 *
 * @see GenericProcessor
 */

// parameter indices
enum
{
    pThreshold,
    pDirection,
    pInputChan,
    pEventChan,
    pEventDur,
    pTimeout,
    pNumPrev,
    pFracPrev,
    pNumNext,
    pFracNext
};

enum CrossingDirection { dNone, dPos, dNeg, dPosOrNeg };

class CrossingDetector : public GenericProcessor
{
public:
    CrossingDetector();
    ~CrossingDetector();
    
	bool hasEditor() const { return true; }

    AudioProcessorEditor* createEditor() override;

	void createEventChannels() override;

    void process(AudioSampleBuffer& continuousBuffer) override;

    void setParameter(int parameterIndex, float newValue) override;

    bool disable() override;

    // getters for use in editor controls
    float getThreshold();
    int getEventDuration();
    int getTimeout();
    float getFracPrev();
    int getNumPrev();
    float getFracNext();
    int getNumNext();

	// start values for parameters
	const CrossingDirection START_DIRECTION = dPos;
	const float START_THRESH = 0.0f;
	const float START_FRAC_PREV = 1.0F;
	const int START_NUM_PREV = 1;
	const float START_FRAC_NEXT = 1.0F;
	const int START_NUM_NEXT = 1;
	const int START_INPUT = 0;
	const int START_OUTPUT = 0;
	const int START_DURATION = 100;
	const int START_TIMEOUT = 1000;

	// limits on numprev / numnext
	// (setting these too high could cause events near the end of a buffer to be significantly delayed,
	// plus we don't want them to exceed the length of a processing buffer)
	const int MAX_NUM_PREV = 20;
	const int MAX_NUM_NEXT = 20;

private:

    // -----utility func.--------
    // Whether there should be a trigger at sample t0, where t0 may be negative (interpreted in relation to the end of prevBuffer)
    // nSamples is the number of samples in the current buffer, determined within the process function.
    // dir is the crossing direction(s) (see #defines above) (must be explicitly specified)
    // uses passed nPrev and nNext rather than the member variables numPrev and numNext.
    bool shouldTrigger(const float* rpCurr, int nSamples, int t0, CrossingDirection dir, int nPrev, int nNext);

	// event-related metadata functions - need to use the same descriptors in the same order.
	void addEventMetaDataFields(EventChannel* chan);
	void populateMetaDataArray(EventChannel* chan, MetaDataValueArray& mdArray, float eventLevel) const;

    // ------parameters------------

    float threshold;

    CrossingDirection direction;

    int inputChan;

    int eventChan;

    // temporary storage of event that must be shut off; allows eventChan to be adjusted during acquisition
    int shutoffChan; 

    int eventDuration; // in samples

    // number of samples after an event onset which may not trigger another event.
    int timeout;

    /* number of previous and next (including current) samples to look at at each timepoint, and fraction required to be above / below threshold
     * generally, things get messy if we try to look too far back or especially forward compared to the size of the processing buffers
     *
     * if numNext samples are not available to look ahead from a timepoint, the test is delayed until the next processing cycle, and if it succeeds,
     * the event occurs on the first sample of the next buffer. thus, setting numNext too large will delay some events slightly.
     */
    float fracPrev;
    int numPrev;
    float fracNext;
    int numNext;

    // ------internals-----------

    // holds on to the previous processing buffer
    Array<float> lastBuffer;

    // the next time at which the event channel should turn off, measured in samples
    // past the start of the current processing buffer. -1 if there is no scheduled shutoff.
    int sampsToShutoff;

    // the next time at which the detector should be reenabled after a timeout period, measured in
    // samples past the start of the current processing buffer. Less than -numNext if there is no scheduled reenable (i.e. the detector is enabled).
    int sampsToReenable;

    EventChannel* eventChannelPtr;
    MetaDataDescriptorArray eventMetaDataDescriptors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrossingDetector);
};

#endif // CROSSING_DETECTOR_H_INCLUDED
