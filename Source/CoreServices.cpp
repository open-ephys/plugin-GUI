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
#include "Processors/Events/Event.h"


using namespace AccessClass;


namespace CoreServices
{
	void updateSignalChain(GenericEditor* source)
	{
		getProcessorGraph()->updateSettings(source->getProcessor());
	}

	void saveRecoveryConfig()
	{
		File configsDir = getSavedStateDirectory();
		if (!configsDir.getFullPathName().contains("plugin-GUI" + File::getSeparatorString() + "Build"))
			configsDir = configsDir.getChildFile("configs-api" + String(PLUGIN_API_VER));

		EditorViewport* ev = getEditorViewport();
		File recoveryFile = configsDir.getChildFile("recoveryConfig.xml");
		ev->saveState(recoveryFile);
	}

	void loadSignalChain(String path)
	{
		if (File(path).existsAsFile())
		{
			getEditorViewport()->loadState(File(path));
		}
	}

	bool getAcquisitionStatus()
	{
		return getControlPanel()->getAcquisitionState();
	}

	void setAcquisitionStatus(bool enable)
	{
		const MessageManagerLock mml;
		getControlPanel()->setAcquisitionState(enable);
	}

	bool getRecordingStatus()
	{
		return getControlPanel()->getRecordingState();
	}

	void setRecordingStatus(bool enable)
	{
		const MessageManagerLock mml;
		getControlPanel()->setRecordingState(enable, true); // starts recording regardless of sync status
	}

	void sendStatusMessage(const String& text)
	{
		LOGD("CoreServices::sendStatusMessage: ", text);
		getBroadcaster()->sendActionMessage(text);
	}

	void sendStatusMessage(const char* text)
	{
		LOGD("CoreServices::sendStatusMessage: ", String(text));
		getBroadcaster()->sendActionMessage(text);
	}

	void highlightEditor(GenericEditor* ed)
	{
		getEditorViewport()->makeEditorVisible(ed);
	}

	juce::int64 getGlobalTimestamp()
	{
		return getProcessorGraph()->getGlobalTimestamp();
	}

	String getGlobalTimestampSource()
	{
		return getProcessorGraph()->getGlobalTimestampSource();
	}

	float getGlobalSampleRate()
	{
		return getProcessorGraph()->getGlobalSampleRate();
	}

	juce::int64 getSoftwareTimestamp()
	{
		return Time::currentTimeMillis();
	}

	float getSoftwareSampleRate()
	{
		return 1000.0f;
	}

	void setRecordingParentDirectory(String dir)
	{
		if (File(dir).exists())
		{
			getControlPanel()->setRecordingParentDirectory(dir);
		}
		else {
			sendStatusMessage(dir + " not found.");
		}
	}

	File getRecordingParentDirectory()
	{
		return getControlPanel()->getRecordingParentDirectory();
	}

	void setRecordingDirectoryBaseText(String text)
	{
		getControlPanel()->setRecordingDirectoryBaseText(text);
	}

	String getRecordingDirectoryBaseText()
	{
		return getControlPanel()->getRecordingDirectoryBaseText();
	}

	String getRecordingDirectoryName()
	{
		return getControlPanel()->getRecordingDirectoryName();
	}

	void createNewRecordingDirectory()
	{
		getControlPanel()->createNewRecordingDirectory();
	}

	void setRecordingDirectoryPrependText(String text)
	{
		getControlPanel()->setRecordingDirectoryPrependText(text);
	}

	void setRecordingDirectoryAppendText(String text)
	{
		getControlPanel()->setRecordingDirectoryAppendText(text);
	}

	String getRecordingDirectoryPrependText()
	{
		return getControlPanel()->getRecordingDirectoryPrependText();
	}

	String getRecordingDirectoryAppendText()
	{
		return getControlPanel()->getRecordingDirectoryAppendText();
	}
	
	std::vector<RecordEngineManager*> getAvailableRecordEngines()
	{
		return getControlPanel()->getAvailableRecordEngines();
	}

	String getDefaultRecordEngineId()
	{
		return getControlPanel()->getSelectedRecordEngineId();
	}

	bool setDefaultRecordEngine(String id)
	{
		return getControlPanel()->setSelectedRecordEngineId(id);
	}

    bool allRecordNodesAreSynchronized()
    {
        for (auto node : getProcessorGraph()->getRecordNodes())
        {
            if (!node->isSynchronized())
                return false;
        }
        
        return true;
    }

	Array<int> getAvailableRecordNodeIds()
	{

		Array<int> nodeIds;

		for (auto node : getProcessorGraph()->getRecordNodes())
		{
			nodeIds.add(node->getNodeId());
		}

		return nodeIds;
	}

	namespace RecordNode
	{

		void setRecordingDirectory(String dir, int nodeId, bool applyToAll)
		{
			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId || applyToAll)
					static_cast<RecordNodeEditor*>(node->getEditor())->setDataDirectory(dir);
			}
		}

		File getRecordingDirectory(int nodeId)
		{

			File directory;

			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId)
					directory = node->getDataDirectory();
			}

			return directory;
		}

		float getFreeSpaceAvailable(int nodeId)
		{

			float freeSpace = -1.0f;

			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId)
					freeSpace = node->getFreeSpaceKilobytes();
			}

			return freeSpace;
		}

		String getRecordEngineId(int nodeId)
		{
			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId)
					return node->getEngineId();
			}
			return String("NO_MATCHING_NODE_FOUND");
		}

		void setRecordEngine(String id, int nodeId, bool applyToAll)
		{
			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId || applyToAll)
					node->setEngine(id);
			}
		}

		int getRecordingNumber(int nodeId)
		{
			int lastRecordingNum = -1;

			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId)
					lastRecordingNum = node->getRecordingNumber();
			}

			return lastRecordingNum;
		}

		int getExperimentNumber(int nodeId)
		{
			
			int experimentNumber = -1;

			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId)
					experimentNumber = node->getExperimentNumber();
			}

			return experimentNumber;
		}


		void createNewRecordingDirectory(int nodeId)
		{
			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId)
					node->createNewDirectory();
			}
		}
    
        bool isSynchronized(int nodeId)
        {
			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId)
					return node->isSynchronized();
			}
			return false;
        }

		/* NOT YET IMPLEMENTED -- these functions are currently global only

		void setRecordingStatus(int nodeId, bool status)
		{
			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId)
				{
					if (status && !getRecordingStatus(nodeId))
					{
						node->startRecording();
						return;
					}

					if (!status && getRecordingStatus(nodeId))
					{
						node->stopRecording();
						return;
					}
				}
			}
		}

		bool getRecordingStatus(int nodeId)
		{
			bool status = false;

			for (auto* node : getProcessorGraph()->getRecordNodes())
			{
				if (node->getNodeId() == nodeId)
					status = node->getRecordingStatus();
			}

			return status;
		}*/

	};

	//const char* getApplicationResource(const char* name, int& size)
	//{
	//	return BinaryData::getNamedResource(name, size);
	//}

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
    	String appDir = File::getSpecialLocation(File::currentApplicationFile).getFullPathName();
		File dir;
		if(appDir.contains("plugin-GUI\\Build\\"))
			dir = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory();
		else
			dir = File::getSpecialLocation(File::commonApplicationDataDirectory).getChildFile("Open Ephys");
#else
		String appDir = File::getSpecialLocation(File::currentApplicationFile).getFullPathName();
		File dir;
		if(appDir.contains("plugin-GUI/Build/"))
			dir = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory();
		else
			dir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("open-ephys");
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


	namespace PluginInstaller
	{
		bool installPlugin(String plugin, String version)
		{
			getUIComponent()->getPluginInstaller()->installPluginAndDependency(plugin, version);

			return true;
		}
	}

};
