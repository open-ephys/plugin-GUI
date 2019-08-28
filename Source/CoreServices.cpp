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


#include "CoreServices.h"
#include "AccessClass.h"

#include "Processors/ProcessorGraph/ProcessorGraph.h"
#include "Processors/RecordNode/RecordNode.h"
#include "UI/EditorViewport.h"
#include "UI/ControlPanel.h"
#include "Processors/MessageCenter/MessageCenterEditor.h"
#include "Processors/Events/Events.h"


using namespace AccessClass;


namespace CoreServices
{
	void updateSignalChain(GenericEditor* source)
	{
		getEditorViewport()->makeEditorVisible(source, false, true);
	}

	bool getRecordingStatus()
	{
		return getControlPanel()->recordButton->getToggleState();
	}

	void setRecordingStatus(bool enable)
	{
		getControlPanel()->setRecordState(enable);
	}

	bool getAcquisitionStatus()
	{
		return getControlPanel()->getAcquisitionState();
	}

	void setAcquisitionStatus(bool enable)
	{
		getControlPanel()->setAcquisitionState(enable);
	}

	void sendStatusMessage(const String& text)
	{
		getBroadcaster()->sendActionMessage(text);
	}

	void sendStatusMessage(const char* text)
	{
		getBroadcaster()->sendActionMessage(text);
	}

	void highlightEditor(GenericEditor* ed)
	{
		getEditorViewport()->makeEditorVisible(ed);
	}

	juce::int64 getGlobalTimestamp()
	{
		return getProcessorGraph()->getGlobalTimestamp(false);
	}

	juce::uint32 getGlobalTimestampSourceFullId()
	{
		return getProcessorGraph()->getGlobalTimestampSourceFullId();
	}

	juce::int64 getSoftwareTimestamp()
	{
		return getProcessorGraph()->getGlobalTimestamp(true);
	}

	float getGlobalSampleRate()
	{
		return getProcessorGraph()->getGlobalSampleRate(false);
	}

	float getSoftwareSampleRate()
	{
		return getProcessorGraph()->getGlobalSampleRate(true);
	}

	void setRecordingDirectory(String dir)
	{
		getControlPanel()->setRecordingDirectory(dir);
	}

	void createNewRecordingDir()
	{
		getControlPanel()->labelTextChanged(NULL);
	}

	void setPrependTextToRecordingDir(String text)
	{
		getControlPanel()->setPrependText(text);
	}

	void setAppendTextToRecordingDir(String text)
	{
		getControlPanel()->setAppendText(text);
	}

	String getSelectedRecordEngineId()
	{
		return getControlPanel()->getSelectedRecordEngineId();
	}

	bool setSelectedRecordEngineId(String id)
	{
		return getControlPanel()->setSelectedRecordEngineId(id);
	}

	namespace RecordNode
	{
		void createNewrecordingDir()
		{
			getProcessorGraph()->getRecordNode()->createNewDirectory();
		}

		File getRecordingPath()
		{
			return getProcessorGraph()->getRecordNode()->getDataDirectory();
		}

		int getRecordingNumber()
		{
			return getProcessorGraph()->getRecordNode()->getRecordingNumber();
		}

		int getExperimentNumber()
		{
			return getProcessorGraph()->getRecordNode()->getExperimentNumber();
		}

		void writeSpike(const SpikeEvent* spike, const SpikeChannel* chan)
		{
			getProcessorGraph()->getRecordNode()->writeSpike(spike, chan);
		}

		void registerSpikeSource(GenericProcessor* processor)
		{
			getProcessorGraph()->getRecordNode()->registerSpikeSource(processor);
		}

		int addSpikeElectrode(const SpikeChannel* elec)
		{
			return getProcessorGraph()->getRecordNode()->addSpikeElectrode(elec);
		}

	};

	const char* getApplicationResource(const char* name, int& size)
	{
		return BinaryData::getNamedResource(name, size);
	}

	File getDefaultUserSaveDirectory()
	{
#if defined(__APPLE__)
		File dir = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("open-ephys");
		if (!dir.isDirectory()) {
			dir.createDirectory();
		}
		return std::move(dir);
#else
		return File::getCurrentWorkingDirectory();
#endif
	}

	String getGUIVersion()
	{
#define XSTR_DEF(s) #s
#define STR_DEF(s) XSTR_DEF(s)
		return STR_DEF(JUCE_APP_VERSION);
	}
};