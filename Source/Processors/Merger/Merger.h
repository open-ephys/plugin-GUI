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

#ifndef __MERGER_H_ED548E77__
#define __MERGER_H_ED548E77__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"

#include <stdio.h>


/**

  Allows the user to merge two signal chains.

  This processor doesn't modify the data passing through it. In fact,
  it has no incoming or outgoing connections. It just allows the outputs from
  TWO source nodes to be connected to ONE destination.

  @see GenericProcessor, ProcessorGraph

*/

class Merger : public GenericProcessor
{
public:

    /** Constructor*/
    Merger();

    /** Destructor */
    ~Merger();

    /** Create the Merger's custom editor */
    AudioProcessorEditor* createEditor();

    /** Nothing happens here, because Mergers are not part of the ProcessorGraph. */
    void process(AudioSampleBuffer& buffer) override {}

    /** Selects which input streams are connected to the output. */
    void updateSettings() override;

    /** Called during updateSettings(), once for each input processor*/
    void addSettingsFromSourceNode(GenericProcessor* sn);

    /** Checks whether or not a particular stream should be sent to the Merger output */
    bool checkStream(const DataStream* stream);

    /** Set the currently displayed path (0 or 1) */
    void switchIO(int) override;

    /** Get the currently displayed path (0 or 1) */
    int getPath() { return activePath;}

    /** Switch the currently displayed path */
    void switchIO() override;

    /** Switches the currently viewed path to a particular input processor*/
    int switchToSourceNode(GenericProcessor* sn);

    /** Sets the source node for the currently selected path*/
    void setMergerSourceNode(GenericProcessor* sn) override;

    /** Returns the source node for a particular path (0 or 1)*/
    GenericProcessor* getSourceNode(int);

    /** Called while loading the signal chain */
    void restoreConnections();

    /** Returns true if at least one source node is connected and enabled*/
    bool stillHasSource() const override;

    /** Saves Merger parameters to XML file*/
    void saveCustomParametersToXml(XmlElement* parentElement) override;

    /** Loads Merger parameters from XML file*/
    void loadCustomParametersFromXml(XmlElement* xml) override;

    /** Returns true if the Merger transmits continuous data for a particular source node*/
    bool sendContinuousForSource(GenericProcessor* sn);

    /** Returns true if the Merger transmits event data for a particular source node*/
    bool sendEventsForSource(GenericProcessor* sn);

    bool mergeEventsA, mergeContinuousA, mergeEventsB, mergeContinuousB;

    GenericProcessor* sourceNodeA;
    GenericProcessor* sourceNodeB;

private:

    int activePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Merger);

};




#endif  // __MERGER_H_ED548E77__
