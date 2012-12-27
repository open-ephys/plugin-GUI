/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#ifndef __AUDIONODE_H_AF61F3C5__
#define __AUDIONODE_H_AF61F3C5__


#ifdef WIN32
#include <Windows.h>
#endif
#include "../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>

#include "GenericProcessor.h"
#include "Editors/AudioEditor.h"

#include "Channel.h"

class AudioEditor;

/**

  The default processor for sending output to the audio monitor.

  The ProcessorGraph has two default nodes: the AudioNode and the RecordNode.
  Every channel of every processor (that's not a sink or a utility) is automatically
  connected to both of these nodes. The AudioNode is used to filter out channels to be
  sent to the audio output device, which can be selected by the user through the AudioEditor
  (located in the ControlPanel). 

  Since the AudioNode exists no matter what, it doesn't appear in the ProcessorList.
  Instead, it's created by the ProcessorGraph at startup.

  Each processor has an "Audio" tab within its channel-selector drawer that determines
  which channels will be monitored. At the moment's there's no centralized way to 
  control the channels going to the audio monitor; it all happens in a distributed
  way through the individual processors.

  @see GenericProcessor, AudioEditor

*/

class AudioNode : public GenericProcessor
{
public:
	
	AudioNode();
	~AudioNode();


	/** Handle incoming data and decide which channels to monitor
  */  
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);

  /** Used to change audio monitoring parameters (such as channels to monitor and volume)
  while acquisition is active.
  */
	void setParameter (int parameterIndex, float newValue);

  /** Creates the AudioEditor (located in the ControlPanel). */
	AudioProcessorEditor* createEditor();

  /** Sets the current channel (in advance of a parameter change). */
  void setChannel(Channel* ch);

  /** Used to turn audio monitoring on and off for individual channels. */
  void setChannelStatus(Channel* ch, bool status);

  /** Resets the connections prior to a new round of data acquisition. */
  void resetConnections();

  /** Resets the connections prior to a new round of data acquisition. */
  void enableCurrentChannel(bool);

  /** Establishes a connection between a channel of a GenericProcessor and the AudioNode. */
  void addInputChannel(GenericProcessor* source, int chan);

  /** A pointer to the AudioNode's editor. */
  ScopedPointer<AudioEditor> audioEditor;
	
private:

	Array<int> leftChan;
	Array<int> rightChan;
	float volume;

  /** An array of pointers to the channels that feed into the AudioNode. */
  Array<Channel*> channelPointers;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioNode);

};





#endif  // __AUDIONODE_H_AF61F3C5__
