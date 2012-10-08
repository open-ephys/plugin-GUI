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

#ifndef __RECORDNODE_H_FB9B1CA7__
#define __RECORDNODE_H_FB9B1CA7__


#include "../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>
#include <map>

#include "GenericProcessor.h"

/**

  Receives inputs from all processors that want to save their data.
  Writes data to disk using fwrite.

  Receives a signal from the ControlPanel to begin recording.

  @see GenericProcessor, ControlPanel

*/

class RecordNode : public GenericProcessor,
                   public FilenameComponentListener
{
public:
	
	RecordNode();
	~RecordNode();
	
	void process(AudioSampleBuffer &buffer, MidiBuffer &eventBuffer, int& nSamples);

	void setParameter (int parameterIndex, float newValue);

  void addInputChannel(GenericProcessor* sourceNode, int chan);

	bool enable();
	bool disable();

  /** Called by the ControlPanel to determine the amount of space
      left in the current dataDirectory.
  */
	float getFreeSpace();

  void setChannel(int id, int chan);

  void setChannelStatus(int chan, bool status);

  void resetConnections();

  bool isAudioOrRecordNode() {return true;}

  void filenameComponentChanged(FilenameComponent*);

  void createNewDirectory();
	
private:

  /** Keep the RecordNode informed of acquisition and record states. 
  */
	bool isRecording, isProcessing, signalFilesShouldClose;

  /** User-selectable directory for saving data files. Currently
      defaults to the user's home directory. 
  */
  File dataDirectory;

  /** Automatically generated folder for each recording session. 
  */
  File rootFolder;

  /** Determines whether a new rootFolder is created when recording
      begins. 
  */
  bool newDataFolder;

  /** Holds data that has been converted from float to int16 before
      saving. 
  */
  int16* continuousDataBuffer;

  /** Integer timestamp saved for each buffer. 
  */ 
  int64 timestamp;

  /** Used to generate timestamps if none are given. 
  */ 
  Time timer;

  /** Holds information for a given channel to be recorded to 
      its own file.
  */ 
  struct Channel
  {
    int nodeId;
    int chan;
    String name;
    bool isRecording;
    String filename;
    FILE* file;
  };

  void closeAllFiles();

  /** Map of continuous channels. 
  */ 
  std::map<int, Channel> continuousChannels;

  /** Map of event channels. 
  */ 
  std::map<int, std::map<int,Channel> > eventChannels;

  /** Method for writing continuous buffers to disk. 
  */ 
  void writeContinuousBuffer(float* data, int nSamples, int channel);
  
  /** Method for writing event buffers to disk. 
  */ 
  void writeEventBuffer(MidiMessage& event, int node, int channel);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordNode);

};



#endif  // __RECORDNODE_H_FB9B1CA7__
