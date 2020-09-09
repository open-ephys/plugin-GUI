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

	File getRecordingDirectory()
	{
		return getControlPanel()->getRecordingDirectory();
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

	std::vector<RecordEngineManager*> getAvailableRecordEngines()
	{
		return getControlPanel()->getAvailableRecordEngines();
	}

	String getSelectedRecordEngineId()
	{
		return getControlPanel()->getSelectedRecordEngineId();
	}

	bool setSelectedRecordEngineId(String id)
	{
		return getControlPanel()->setSelectedRecordEngineId(id);
	}

	int getSelectedRecordEngineIdx()
	{
		return getControlPanel()->recordSelector->getSelectedId();
	}

	namespace RecordNode
	{

		void createNewrecordingDir()
		{
			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				node->createNewDirectory();
			}
		}

		//TODO: This needs to be well-defined...just testing for now P.K.
		int getRecordingNumber()
		{
			int lastRecordingNum = -1;

			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				lastRecordingNum = node->getRecordingNumber();
			}

			return lastRecordingNum;
		}
		
		File getRecordingPath()
		{
			return File();
		}

		int getExperimentNumber()
		{
			
			int experimentNumber = -1;

			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				experimentNumber = node->getExperimentNumber();
			}

			return experimentNumber;
		}

		bool getRecordThreadStatus()
		{
			
			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getRecordThreadStatus())
					return true;
			}

			return false;
		}

		/*
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
		*/

	};

	const char* getApplicationResource(const char* name, int& size)
	{
		return BinaryData::getNamedResource(name, size);
	}

	File getDefaultUserSaveDirectory()
	{
#if defined(__APPLE__)
    	const File dir = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Open Ephys");
#elif _WIN32
    	const File dir = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Open Ephys");
#else
    	const File dir = File::getSpecialLocation(File::userHomeDirectory).getChildFile("open-ephys");
#endif
		if (!dir.isDirectory()) {
			dir.createDirectory();
		}
		return std::move(dir);
	}

	File getSavedStateDirectory() {
#if defined(__APPLE__)
    	File dir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support/open-ephys");
#elif _WIN32
    	File dir = File::getSpecialLocation(File::commonApplicationDataDirectory).getChildFile("Open Ephys");
#else
		File dir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(".open-ephys");;
#endif
		if (!dir.isDirectory()) {
			dir.createDirectory();
		}
    	return std::move(dir);
	}

	String getGUIVersion()
	{
#define XSTR_DEF(s) #s
#define STR_DEF(s) XSTR_DEF(s)
		return STR_DEF(JUCE_APP_VERSION);
	}
};