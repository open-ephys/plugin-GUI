#include "Synchronizer.h"
#include "Utils.h"

Subprocessor::Subprocessor(float expectedSampleRate_)
{
	expectedSampleRate = expectedSampleRate_;
	actualSampleRate = -1.0f;

	startSample = -1;
	lastSample = -1;

	startSampleMasterTime = -1.0f;
	lastSampleMasterTime = -1.0f;

	syncChannel = -1;

	isSynchronized = false;
	receivedEventInWindow = false;
	receivedMasterTimeInWindow = false;

	sampleRateTolerance = 0.01;
}

void Subprocessor::reset()
{

	startSampleMasterTime = -1.0f;
	lastSampleMasterTime = -1.0f;

	isSynchronized = false;

}

void Subprocessor::setMasterTime(float masterTimeSec_)
{
	if (!receivedMasterTimeInWindow)
	{
		tempMasterTime = masterTimeSec_;
		receivedMasterTimeInWindow = true;
	}
	else { // multiple events, something could be wrong
		receivedMasterTimeInWindow = false;
	}

}

void Subprocessor::addEvent(int sampleNumber)
{
	if (!receivedEventInWindow)
	{
		tempSampleNum = sampleNumber;
		receivedEventInWindow = true;
	}
	else { // multiple events, something could be wrong
		receivedEventInWindow = false;
	}

}

void Subprocessor::closeSyncWindow()
{
	if (receivedEventInWindow && receivedMasterTimeInWindow)
	{
		if (startSample < 0)
		{
			startSample = tempSampleNum;
			startSampleMasterTime = tempMasterTime;
		}
		else {

			lastSample = tempSampleNum;
			lastSampleMasterTime = tempMasterTime;

			float expectedIntervalSec = (lastSample - startSample) / expectedSampleRate;
			float ratio = (lastSampleMasterTime - startSampleMasterTime) / expectedIntervalSec;
			float tempSampleRate = expectedSampleRate / ratio;

			if (actualSampleRate < 0.0f)
			{
				actualSampleRate = tempSampleRate;
				isSynchronized = true;
				std::cout << "New sample rate: " << actualSampleRate << std::endl;
			}
			else {
				// check whether the sample rate has changed
				if (abs((tempSampleRate - actualSampleRate) / actualSampleRate) < sampleRateTolerance)
				{
					actualSampleRate = tempSampleRate;
					isSynchronized = true;
					std::cout << "Updated sample rate: " << actualSampleRate << std::endl;
				}
				else { // reset the clock
					startSample = tempSampleNum;
					startSampleMasterTime = tempMasterTime;
					isSynchronized = false;
				}
			}
		}
	}

	//std::cout << "Subprocessor closed sync window." << std::endl;

	receivedEventInWindow = false;
	receivedMasterTimeInWindow = false;
}

// =======================================================

Synchronizer::Synchronizer(RecordNode* parentNode)
{
	syncWindowLengthMs = 50;
	syncWindowIsOpen = false;
	firstMasterSync = true;
	node = parentNode;
}

Synchronizer::~Synchronizer()
{

}

void Synchronizer::reset()
{
	
    syncWindowIsOpen = false;
    firstMasterSync = true;
	eventCount = 0;
    
    std::map<int, std::map<int, Subprocessor*>>::iterator it;
    std::map<int, Subprocessor*>::iterator ptr;
    
    for (it = subprocessors.begin(); it != subprocessors.end(); it++) {
        for (ptr = it->second.begin(); ptr != it->second.end(); ptr++) {
            ptr->second->reset();
        }
    }
}

void Synchronizer::addSubprocessor(int sourceID, int subProcIndex, float expectedSampleRate)
{
	subprocessorArray.add(new Subprocessor(expectedSampleRate));
	subprocessors[sourceID][subProcIndex] = subprocessorArray.getLast();
}

void Synchronizer::setMasterSubprocessor(int sourceID, int subProcIndex)
{
	masterProcessor = sourceID;
	masterSubprocessor = subProcIndex;
	reset();
}

void Synchronizer::setSyncChannel(int sourceID, int subProcIdx, int ttlChannel)
{
	LOGD("Set sync channel: {", sourceID, ",", subProcIdx, "}->", ttlChannel);
	subprocessors[sourceID][subProcIdx]->syncChannel = ttlChannel;
	reset();
}

int Synchronizer::getSyncChannel(int sourceID, int subProcIdx)
{
	LOGD("Get sync channel: {", sourceID, ",", subProcIdx, "}->", subprocessors[sourceID][subProcIdx]->syncChannel);
	return subprocessors[sourceID][subProcIdx]->syncChannel;
}

void Synchronizer::addEvent(int sourceID, int subProcIdx, int ttlChannel, int sampleNumber)
{

	if (subprocessors[sourceID][subProcIdx]->syncChannel == ttlChannel)
	{
		
		if (!syncWindowIsOpen)
		{
			openSyncWindow();
		}

		subprocessors[sourceID][subProcIdx]->addEvent(sampleNumber);

		if (sourceID == masterProcessor && subProcIdx == masterSubprocessor)
		{

			LOGD("Got event on master!");

			float masterTimeSec;

			if (!firstMasterSync)
			{
				masterTimeSec = (sampleNumber - subprocessors[sourceID][subProcIdx]->startSample) / subprocessors[sourceID][subProcIdx]->expectedSampleRate;
			}
			else
			{
				masterTimeSec = 0.0f;
				firstMasterSync = false;
			}

			std::map<int, std::map<int, Subprocessor*>>::iterator it;
			std::map<int, Subprocessor*>::iterator ptr;

			for (it = subprocessors.begin(); it != subprocessors.end(); it++) 
			{
				for (ptr = it->second.begin(); ptr != it->second.end(); ptr++) 
				{
					ptr->second->setMasterTime(masterTimeSec);
				}
			}

			if (eventCount % 10 == 0)
				std::cout << "Master time: " << masterTimeSec << std::endl;

			eventCount++;
		}
	}
}

float Synchronizer::convertTimestamp(int sourceID, int subProcID, int sampleNumber)
{

	if (subprocessors[sourceID][subProcID]->isSynchronized)
	{
		return (sampleNumber - subprocessors[sourceID][subProcID]->startSample) /
			subprocessors[sourceID][subProcID]->actualSampleRate +
			subprocessors[sourceID][subProcID]->startSampleMasterTime;
	}
	else {
		return -1.0f;
	}
}

void Synchronizer::openSyncWindow()
{
	startTimer(syncWindowLengthMs);

	syncWindowIsOpen = true;
}

bool Synchronizer::isSubprocessorSynced(int id, int idx)
{
	return subprocessors[id][idx]->isSynchronized;
}

SyncStatus Synchronizer::getStatus(int id, int idx)
{

	//Deal with synchronization of spikes and events later...
	if (id < 0)
		return SyncStatus::OFF;

	if (isSubprocessorSynced(id, idx))
		return SyncStatus::SYNCED;
	else if (true)
		return SyncStatus::SYNCING;
	else
		return SyncStatus::OFF;

}

void Synchronizer::hiResTimerCallback()
{
	stopTimer();

	syncWindowIsOpen = false;

	std::map<int, std::map<int, Subprocessor*>>::iterator it;
	std::map<int, Subprocessor*>::iterator ptr;

	for (it = subprocessors.begin(); it != subprocessors.end(); it++) {
		for (ptr = it->second.begin(); ptr != it->second.end(); ptr++) {
			ptr->second->closeSyncWindow();
		}
	}
}