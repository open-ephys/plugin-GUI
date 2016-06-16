#include "ContinuousCircularBuffer.h"

void ContinuousCircularBuffer::reallocate(int NumCh)
{
	numCh = NumCh;
	Buf.resize(numCh);
	for (int k = 0; k< numCh; k++)
	{
		Buf[k].resize(bufLen);
	}
	numSamplesInBuf = 0;
	ptr = 0; // points to a valid position in the buffer.

}


ContinuousCircularBuffer::ContinuousCircularBuffer(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer)
{
	Time t;

	numTicksPerSecond = (double)t.getHighResolutionTicksPerSecond();

	int numSamplesToHoldPerChannel = (int)(SamplingRate * NumSecInBuffer / SubSampling);
	buffer_dx = 1.0 / (SamplingRate / SubSampling);
	subSampling = SubSampling;
	samplingRate = SamplingRate;
	numCh = NumCh;
	leftover_k = 0;
	Buf.resize(numCh);


	for (int k = 0; k< numCh; k++)
	{
		Buf[k].resize(numSamplesToHoldPerChannel);
	}

	hardwareTS.resize(numSamplesToHoldPerChannel);
	softwareTS.resize(numSamplesToHoldPerChannel);
	valid.resize(numSamplesToHoldPerChannel);
	bufLen = numSamplesToHoldPerChannel;
	numSamplesInBuf = 0;
	ptr = 0; // points to a valid position in the buffer.
}


void ContinuousCircularBuffer::update(int channel, int64 hardware_ts, int64 software_ts, bool rise)
{
	// used to record ttl pulses as continuous data...
	mut.enter();
	valid[ptr] = true;
	hardwareTS[ptr] = hardware_ts;
	softwareTS[ptr] = software_ts;

	Buf[channel][ptr] = (rise) ? 1.0f : 0.0f;

	ptr++;
	if (ptr == bufLen)
	{
		ptr = 0;
	}
	numSamplesInBuf++;
	if (numSamplesInBuf >= bufLen)
	{
		numSamplesInBuf = bufLen;
	}
	mut.exit();
}

void ContinuousCircularBuffer::update(AudioSampleBuffer& buffer, int64 hardware_ts, int64 software_ts, int numpts)
{
	mut.enter();

	// we don't start from zero because of subsampling issues.
	// previous packet may not have ended exactly at the last given sample.
	int k = leftover_k;
	int lastUsedSample=0;
	for (; k < numpts; k += subSampling)
	{
		lastUsedSample = k;
		valid[ptr] = true;
		hardwareTS[ptr] = hardware_ts + k;
		softwareTS[ptr] = software_ts + int64(float(k) / samplingRate * numTicksPerSecond);

		for (int ch = 0; ch < numCh; ch++)
		{
			Buf[ch][ptr] = *(buffer.getReadPointer(ch, k));
		}
		ptr++;
		if (ptr == bufLen)
		{
			ptr = 0;
		}
		numSamplesInBuf++;
		if (numSamplesInBuf >= bufLen)
		{
			numSamplesInBuf = bufLen;
		}
	}

	int numMissedSamples = (numpts - 1) - lastUsedSample;
	leftover_k = (subSampling - numMissedSamples - 1) % subSampling;
	mut.exit();

}


void ContinuousCircularBuffer::update(std::vector<std::vector<bool>> contdata, int64 hardware_ts, int64 software_ts, int numpts)
{
	mut.enter();

	// we don't start from zero because of subsampling issues.
	// previous packet may not have ended exactly at the last given sample.
	int k = leftover_k;
	int lastUsedSample;
	for (; k < numpts; k += subSampling)
	{
		lastUsedSample = k;
		valid[ptr] = true;
		hardwareTS[ptr] = hardware_ts + k;
		softwareTS[ptr] = software_ts + int64(float(k) / samplingRate * numTicksPerSecond);

		for (int ch = 0; ch < numCh; ch++)
		{
			Buf[ch][ptr] = contdata[ch][k];
		}
		ptr++;
		if (ptr == bufLen)
		{
			ptr = 0;
		}
		numSamplesInBuf++;
		if (numSamplesInBuf >= bufLen)
		{
			numSamplesInBuf = bufLen;
		}
	}

	int numMissedSamples = (numpts - 1) - lastUsedSample;
	leftover_k = subSampling - numMissedSamples - 1;
	mut.exit();

}
/*
void ContinuousCircularBuffer::AddDataToBuffer(std::vector<std::vector<double>> lfp, double soft_ts)
{
mut.enter();
int numpts = lfp[0].size();
for (int k = 0; k < numpts / subSampling; k++)
{
valid[ptr] = true;
for (int ch = 0; ch < numCh; ch++)
{
Buf[ch][ptr] = lfp[ch][k];
TS[ptr] = soft_ts + (double)(k * subSampling) / samplingRate;
}
ptr++;
if (ptr == bufLen)
{
ptr = 0;
}
numSamplesInBuf++;
if (numSamplesInBuf >= bufLen)
{
numSamplesInBuf = bufLen;
}
}
mut.exit();
}
*/

int ContinuousCircularBuffer::GetPtr()
{
	return ptr;
}

/************************************************************/



