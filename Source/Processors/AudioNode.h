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

class AudioEditor;

/**

  Selects which channels to send to the audio monitor. Controls output volume.

  @see GenericProcessor, AudioEditor

*/

class AudioNode : public GenericProcessor
{
public:
	
	// real member functions:
	AudioNode();
	~AudioNode();
	
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);
	void setParameter (int parameterIndex, float newValue);

	AudioProcessorEditor* createEditor();

  void setChannelStatus(int, bool);

 // bool isAudioOrRecordNode() {return true;}

  void enableCurrentChannel(bool);

   // AudioEditor* getEditor() {return audioEditor;}

    ScopedPointer<AudioEditor> audioEditor;
	
private:

	Array<int> leftChan;
	Array<int> rightChan;
	float volume;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioNode);

};





#endif  // __AUDIONODE_H_AF61F3C5__
