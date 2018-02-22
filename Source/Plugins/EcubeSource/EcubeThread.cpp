/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Michael Borisov

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
#include "EcubeThread.h"
#include "EcubeDialogComponent.h"
#include <stdint.h>

#ifdef ECUBE_COMPILE
#import "libid:60C0AAC2-1E0B-4FE5-A921-AF9CEEAAA582"

using namespace ecubeapiLib;

class EcubeDevInt
{
public:

    enum DataFormat
    {
        dfSeparateChannelsAnalog,
        dfInterleavedChannelsAnalog,
        dfDigital
    };
    IEcubePtr pEcube;
    IEcubeDevicePtr pDevice;
    IEcubeModulePtr pModule;
    IEcubeSpeakerPtr pSpeaker;
    std::vector<IEcubeChannelPtr> vpChannels;
    IEcubeChannelPtr pSpeakerChannel;
    unsigned n_channel_objects;
    std::map<int, int> chid_map;
    IEcubeAnalogAcquisitionPtr pStrmA;
    IEcubeDigitalInputStreamingPtr pStrmD;
    HeapBlock<float, true> interleaving_buffer;
    HeapBlock<uint64_t, true> event_buffer;
    HeapBlock<uint32_t, true> bit_conversion_tables;
    bool buf_timestamp_locked;
    unsigned long buf_timestamp;
    int64 buf_timestamp64;
    unsigned long int_buf_size;
    DataFormat data_format;
    unsigned long sampletime_80mhz;
};

static const char bits_port0[16] = { 23, 22, -1, 14, 11, -1, -1, 28, 12, 10, 27, 26, -1, -1, -1, -1 };
static const char bits_port1[16] = { 20, 21, 19, 18, 13, 6, 4, 5, 3, 2, -1, -1, 29, -1, 24, 25 };
static const char bits_port2[16] = { 16, 17, 15, 8, 9, 7, 0, 1, 31, 30, -1, -1, -1, -1, -1, -1 };

// Builds bit conversion table for 8 bits of raw data
void build_bit_conversion_table(uint32_t* table, const char* bits)
{
    for (uint16_t i = 0; i < 256; i++)
    {
        uint8_t is = i;
        uint32_t outval = 0;
        for (uint16_t j = 0; j < 8; j++)
        {
            if ((is & 1) && bits[j]>=0)
            {
                outval |= (uint32_t)1<<bits[j];
            }
            is >>= 1;
        }
        table[i] = outval;
    }
}

void build_bit_conversion_tables(uint32_t* tables)
{
    // Bit conversion tables have 256 uint64s for each 8 bits of ecube ports
    // Each sparse 16-bit port has two such tables
    // The structure in memory is {tbl_port0l, tbl_port0h, tbl_port1l, tbl_port1h, tbl_port2l, tbl_port2h)
    build_bit_conversion_table(tables        , bits_port0);
    build_bit_conversion_table(tables + 0x100, bits_port0 + 8);
    build_bit_conversion_table(tables + 0x200, bits_port1);
    build_bit_conversion_table(tables + 0x300, bits_port1 + 8);
    build_bit_conversion_table(tables + 0x400, bits_port2);
    build_bit_conversion_table(tables + 0x500, bits_port2 + 8);
}

static std::vector<std::wstring> SafeArrayToVecStr(SAFEARRAY* sa)
{
    HRESULT hr;
    long lbound, ubound;

    if (SafeArrayGetElemsize(sa) != sizeof(BSTR*))
        _com_raise_error(E_FAIL);
    if (SafeArrayGetDim(sa)!=1)
        _com_raise_error(E_FAIL);
    if (FAILED(hr = SafeArrayGetLBound(sa, 1, &lbound)))
        _com_raise_error(hr);
    if (FAILED(hr = SafeArrayGetUBound(sa, 1, &ubound)))
        _com_raise_error(hr);

    std::vector<std::wstring> result;
    for (long index = lbound; index <= ubound; index++)
    {
        BSTR raw_bstr;
        if (FAILED(hr = SafeArrayGetElement(sa, &index, &raw_bstr)))
            _com_raise_error(hr);
        _bstr_t bstr(raw_bstr, false);
        result.push_back(raw_bstr);
    }
    return result;
}

static StringArray SafeArrayToStringArray(SAFEARRAY* sa)
{
    HRESULT hr;
    long lbound, ubound;
    VARTYPE vt;

    hr = SafeArrayGetVartype(sa, &vt);
    if (FAILED(hr))
        _com_raise_error(hr);
    if (vt!=VT_BSTR)
        _com_raise_error(E_FAIL);
    if (SafeArrayGetElemsize(sa) != sizeof(BSTR*))
        _com_raise_error(E_FAIL);
    if (SafeArrayGetDim(sa) != 1)
        _com_raise_error(E_FAIL);
    if (FAILED(hr = SafeArrayGetLBound(sa, 1, &lbound)))
        _com_raise_error(hr);
    if (FAILED(hr = SafeArrayGetUBound(sa, 1, &ubound)))
        _com_raise_error(hr);

    StringArray result;
    for (long index = lbound; index <= ubound; index++)
    {
        BSTR raw_bstr;
        if (FAILED(hr = SafeArrayGetElement(sa, &index, &raw_bstr)))
            _com_raise_error(hr);
        _bstr_t bstr(raw_bstr, false);
        result.add(raw_bstr);
    }
    return result;
}

std::vector<std::wstring> GetEcubeModuleChannels(IEcubeModulePtr& mp)
{
    std::vector<std::wstring> chnames;
    {
        SAFEARRAY* sa = mp->GetChannels();
        chnames = SafeArrayToVecStr(sa);
        SafeArrayDestroy(sa); // ARTEM - Leaks a safearray if an exception is thrown here
    }
    return chnames;
}

EcubeThread::EcubeThread(SourceNode* sn) : DataThread(sn), numberingScheme(1), acquisition_running(false)
{
    try
    {
        EcubeDialogComponent component;
        DialogWindow::LaunchOptions opt;
        opt.dialogTitle = "eCube parameters";
        opt.content = OptionalScopedPointer<Component>(&component, false);
        opt.escapeKeyTriggersCloseButton = true;

        pDevInt = new EcubeDevInt;
        pDevInt->pEcube.CreateInstance(__uuidof(Ecube));
        {
            SAFEARRAY* sa = pDevInt->pEcube->DetectNetworkDevices();
            StringArray a(SafeArrayToStringArray(sa)); // ARTEM - Leaks a safearray if an exception is thrown here
            SafeArrayDestroy(sa);
            component.SetDeviceNames(a);
        }
        int dres = opt.runModal();
        if (dres != 1)
        {
            throw std::runtime_error("Operation cancelled");
        }

        pDevInt->pDevice = pDevInt->pEcube->OpenNetworkDevice(_bstr_t(component.GetAddressValue().toUTF16()));
        pDevInt->pSpeaker = pDevInt->pDevice->OpenModule(_bstr_t(L"AudioMonitorSpeaker"));
        pDevInt->pSpeakerChannel = pDevInt->pSpeaker->OpenChannel(_bstr_t(L"ao1"));
        pDevInt->n_channel_objects = 0;
        {
            String selmod = component.GetModuleName();
            if (selmod == "Headstage(s)")
            {
                m_samplerate = 25000.0;
                pDevInt->sampletime_80mhz = 3200;
                pDevInt->data_format = EcubeDevInt::dfSeparateChannelsAnalog;
                // Get status of headstage selection
                bool selhs[10];
                component.GetHeadstageSelection(selhs);
                bool acq_created = false;
                for (int i = 0; i < 10; i++)
                {
                    if (selhs[i])
                    {
                        String modname = "Headstage" + String(i + 1);
                        pDevInt->pModule = pDevInt->pDevice->OpenModule(_bstr_t(modname.toUTF16()));
                        std::vector<std::wstring> chnames = GetEcubeModuleChannels(pDevInt->pModule);
                        for (int j = 0; j < chnames.size(); j++)
                        {
                            IEcubeChannelPtr pch = pDevInt->pModule->OpenChannel(chnames[j].c_str());
                            if (!acq_created)
                            {
                                pDevInt->pStrmA = pDevInt->pEcube->CreateAnalogAcquisition(pch);
                                acq_created = true;
                            }
                            else
                                pDevInt->pStrmA->AddChannel(pch);
                            pDevInt->chid_map[pch->GetID()] = pDevInt->n_channel_objects;
                            pDevInt->n_channel_objects++;
                            pDevInt->vpChannels.push_back(pch);
                        }
                    }
                }
                sourceBuffers.set(0,new DataBuffer(pDevInt->n_channel_objects, 10000));
                // Create the interleaving buffer based on the number of channels
                pDevInt->interleaving_buffer.malloc(sizeof(float)* 1500 * pDevInt->n_channel_objects);
            }
            else if (selmod == "Panel Analog Input")
            {
                pDevInt->pModule = pDevInt->pDevice->OpenModule(_bstr_t(L"PanelAnalogInput"));
                pDevInt->data_format = EcubeDevInt::dfInterleavedChannelsAnalog;
                bool acq_created = false;
                std::vector<std::wstring> chnames = GetEcubeModuleChannels(pDevInt->pModule);
                for (int j = 0; j < chnames.size(); j++)
                {
                    IEcubeChannelPtr pch = pDevInt->pModule->OpenChannel(chnames[j].c_str());
                    if (!acq_created)
                    {
                        pDevInt->pStrmA = pDevInt->pEcube->CreateAnalogAcquisition(pch);
                        acq_created = true;
                    }
                    else
                        pDevInt->pStrmA->AddChannel(pch);
                    pDevInt->chid_map[pch->GetID()] = pDevInt->n_channel_objects;
                    pDevInt->n_channel_objects++;
                }
                m_samplerate = component.GetSampleRate(); // Initial user-specified sample rate
                pDevInt->pStrmA->PutSampleRate(m_samplerate);
                m_samplerate = pDevInt->pStrmA->GetSampleRate(); // Retrieve the coerced value from the API
                pDevInt->sampletime_80mhz = pDevInt->pStrmA->GetSampleRateDen();
                pDevInt->sampletime_80mhz *= 80000000 / pDevInt->pStrmA->GetSampleRateNum();

                sourceBuffers.set(0,new DataBuffer(32, 10000));
                // The interleaving buffer is there just for short->float conversion
                pDevInt->interleaving_buffer.malloc(sizeof(float)* 1500);
            }
            else if (selmod == "Panel Digital Input")
            {
                pDevInt->pModule = pDevInt->pDevice->OpenModule(_bstr_t(L"PanelDigitalIO"));
                m_samplerate = 25000.0;
                pDevInt->sampletime_80mhz = 3200;
                pDevInt->data_format = EcubeDevInt::dfDigital;

                bool acq_created = false;
                std::vector<std::wstring> chnames = GetEcubeModuleChannels(pDevInt->pModule);
                for (int j = 0; j < chnames.size(); j++)
                {
                    IEcubeChannelPtr pch = pDevInt->pModule->OpenChannel(chnames[j].c_str());
                    if (!acq_created)
                    {
                        pDevInt->pStrmD = pDevInt->pEcube->CreateDigitalInputStreaming(pch);
                        acq_created = true;
                    }
                    else
                        pDevInt->pStrmD->AddChannel(pch);
                    pDevInt->chid_map[pch->GetID()] = pDevInt->n_channel_objects;
                    pDevInt->n_channel_objects++;
                }

                sourceBuffers.set(0,new DataBuffer(64, 10000));
                // Create the interleaving buffer based on the number of digital ports
                pDevInt->interleaving_buffer.malloc(sizeof(float)* 1500 * 64);
                // Create the analog of interleaving buffer in packed format (int64)
                pDevInt->event_buffer.malloc(sizeof(uint64_t)* 1500);
                pDevInt->bit_conversion_tables.malloc(sizeof(uint32_t)* 0x600);
                build_bit_conversion_tables(pDevInt->bit_conversion_tables);
            }
            else
                throw std::runtime_error("Invlid module selection");
        }

        pDevInt->buf_timestamp_locked = false;

        setDefaultChannelNames();

    }
    catch (_com_error& e)
    {
        // Convert COM errors to std::runtime_errors, to avoid making other files Windows-dependent
        throw std::runtime_error(std::string(e.Description()));
    }
}

/* This will give default names & gains to channels, unless they were manually modified by the user
In that case, the query channelModified, will return the values that need to be put */
void EcubeThread::setDefaultChannelNames()
{

    String prefix;
    DataChannel::DataChannelTypes common_type;

    int numch = getNumChannels();

    if (pDevInt->data_format == EcubeDevInt::dfSeparateChannelsAnalog)
    {
        prefix = "HS_CH";
		common_type = DataChannel::HEADSTAGE_CHANNEL;
    }
    else if (pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
    {
        prefix = "PAI";
		common_type = DataChannel::ADC_CHANNEL;
    }
    else //if (pDevInt->data_format == EcubeDevInt::dfDigital)
    {
        prefix = "PDI";
		common_type = DataChannel::ADC_CHANNEL;
    }

    if (numberingScheme != 1)
        prefix += "_";

    for (int i = 0; i < numch; i++)
    {
        ChannelCustomInfo ci;
        ci.name = prefix + String(i);
        ci.gain = getBitVolts(i);
        channelInfo.set(i, ci);
    }

}

bool EcubeThread::usesCustomNames() const
{
    return true;
}

void EcubeThread::setDefaultNamingScheme(int scheme)
{
    numberingScheme = scheme;
    setDefaultChannelNames();
}


EcubeThread::~EcubeThread()
{
    if (acquisition_running)
        stopAcquisition();
    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }
    waitForThreadToExit(-1);
}

int EcubeThread::getNumDataOutputs(DataChannel::DataChannelTypes type, int subIdx) const
{
	if (subIdx != 0) return 0;
	if (type == DataChannel::HEADSTAGE_CHANNEL)
	{
		if (pDevInt->data_format == EcubeDevInt::dfSeparateChannelsAnalog)
			return pDevInt->n_channel_objects;
		else
			return 0;
	}
	else if (type == DataChannel::ADC_CHANNEL)
	{
		if (pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
			return 32;
		else if (pDevInt->data_format == EcubeDevInt::dfDigital)
			return 64;
		else
			return 0;
	}
	else return 0;
}

int EcubeThread::getNumChannels()
{
    if (pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
        return 32;
    else if (pDevInt->data_format == EcubeDevInt::dfDigital)
        return 64;
    else
        return pDevInt->n_channel_objects;
}

int EcubeThread::getNumTTLOutputs(int subIdx) const
{
	if (subIdx != 0) return 0;
    if (pDevInt->data_format == EcubeDevInt::dfDigital)
        return 64;
    else
        return 0;
}

float EcubeThread::getSampleRate(int subIdx) const
{
    return m_samplerate;
}

float EcubeThread::getBitVolts(int chan) const
{
    if (pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog || pDevInt->data_format == EcubeDevInt::dfDigital)
        return 10.0/32768; // Volts per bit for front panel analog input and fictive v/bit for the digital input
    else
        return 6.25e3 / 32768; // Microvolts per bit for the headstage channels
}

float EcubeThread::getBitVolts(const DataChannel* chan) const
{
    if (pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog || pDevInt->data_format == EcubeDevInt::dfDigital)
        return 10.0 / 32768; // Volts per bit for front panel analog input and fictive v/bit for the digital input
    else
        return 6.25e3 / 32768; // Microvolts per bit for the headstage channels
}

bool EcubeThread::foundInputSource()
{
    return true;
}

bool EcubeThread::updateBuffer()
{
    unsigned long ba;
    ttlEventWords.set(0,0);
    int nchan = pDevInt->n_channel_objects;

    if (pDevInt->data_format == EcubeDevInt::dfSeparateChannelsAnalog || pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
        ba = pDevInt->pStrmA->WaitForData(100);
    else
        ba = pDevInt->pStrmD->WaitForData(100);
    while (ba)
    {
        for (unsigned long i = 0; i < ba; i++)
        {
            IEcubeDataBufferPtr ab;
            if (pDevInt->data_format == EcubeDevInt::dfSeparateChannelsAnalog || pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
                ab = pDevInt->pStrmA->FetchNextBuffer();
            else
                ab = pDevInt->pStrmD->FetchNextBuffer();
            unsigned long chid = ab->GetStreamID();
            std::map<int, int>::const_iterator chit = pDevInt->chid_map.find(chid);
            if (chit != pDevInt->chid_map.end())
            {
                unsigned long bts = ab->GetTimestamp();
                unsigned long datasize = ab->GetDataSize() / 2; // Data size is returned in bytes, not in samples

                if (pDevInt->data_format == EcubeDevInt::dfSeparateChannelsAnalog)
                {
                    if (!pDevInt->buf_timestamp_locked || (bts - pDevInt->buf_timestamp >= pDevInt->sampletime_80mhz && pDevInt->buf_timestamp - bts >= pDevInt->sampletime_80mhz)
                        /*bts != pDevInt->buf_timestamp*/
                        || datasize != pDevInt->int_buf_size)
                    {
                        // The new buffer does not match interleaving buffer length, or has a different timestamp,
                        // or interleaving buffer is empty
                        if (pDevInt->buf_timestamp_locked)
                        {
                            // Interleaving buffer is not empty.
                            // Send its contents out to the application
                            int64 cts = pDevInt->buf_timestamp64 / pDevInt->sampletime_80mhz; // Convert eCube 80MHz timestamp into a 25kHz timestamp
                            for (unsigned long j = 0; j < pDevInt->int_buf_size; j++)
                            {
								sourceBuffers[0]->addToBuffer(pDevInt->interleaving_buffer + j*nchan, &cts, &ttlEventWords.getReference(0), 1);
                                cts++;
                            }
                            // Update the 64-bit timestamp, take account of its wrap-around
                            unsigned tsdif = bts - pDevInt->buf_timestamp;
                            pDevInt->buf_timestamp64 += tsdif;
                        }
                        else
                        {
                            // The interleaving buffer is empty
                            pDevInt->buf_timestamp64 = bts;
                        }
                        pDevInt->int_buf_size = datasize;
                        pDevInt->buf_timestamp = bts;
                        pDevInt->buf_timestamp_locked = true;
                        // Clear the interleaving buffer within the new packet's size
                        memset(pDevInt->interleaving_buffer, 0, sizeof(float)*datasize*nchan);
                    }
                    chid = chit->second; // Adjust the channel id to become the channel index
                    unsigned char* dp = ab->GetDataPointer();
                    const short* pData = (const short*)dp;
                    for (unsigned long j = 0; j < datasize; j++)
                    {
                        pDevInt->interleaving_buffer[chid + nchan*j] = pData[j] * 6.25e3 / 32768; // Convert into microvolts
                    }
                }
                else if (pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
                {
                    if (pDevInt->buf_timestamp_locked)
                    {
                        // Update the 64-bit timestamp, take care of its wrap-around
                        unsigned tsdif = bts - pDevInt->buf_timestamp;
                        pDevInt->buf_timestamp64 += tsdif;
                    }
                    else
                    {
                        pDevInt->buf_timestamp64 = bts;
                    }
                    pDevInt->buf_timestamp = bts;
                    pDevInt->buf_timestamp_locked = true;
                    unsigned char* dp = ab->GetDataPointer();
                    const short* pData = (const short*)dp;
                    for (unsigned j = 0; j < datasize; j++)
                    {
                        pDevInt->interleaving_buffer[j] = pData[j] * 10.0/32768; // Convert into volts
                    }
                    unsigned long datasam = datasize / 32;
                    int64 cts = pDevInt->buf_timestamp64 / pDevInt->sampletime_80mhz; // Convert eCube's 80MHz timestamps into number of samples on the Panel Analog input (orig sample rate 1144)
                    for (unsigned long j = 0; j < datasam; j++)
                    {
						sourceBuffers[0]->addToBuffer(pDevInt->interleaving_buffer + j * 32, &cts, &ttlEventWords.getReference(0), 1);
                        cts++;
                    }
                }
                else // Digital data
                {
                    unsigned tsdif = bts - pDevInt->buf_timestamp;
                    if (!pDevInt->buf_timestamp_locked || (bts != pDevInt->buf_timestamp && tsdif!=5 && tsdif!=10 && tsdif!=0xFFFFFFFB && tsdif!=0xFFFFFFFA) || datasize != pDevInt->int_buf_size)
                    {
                        // The new buffer does not match interleaving buffer length, or has a different timestamp,
                        // or interleaving buffer is empty
                        if (pDevInt->buf_timestamp_locked)
                        {
                            // Interleaving buffer is not empty.
                            // Send its contents out to the application
                            int64 cts = pDevInt->buf_timestamp64 / pDevInt->sampletime_80mhz; // Convert eCube 80MHz timestamp into a 25kHz timestamp
                            for (unsigned long j = 0; j < pDevInt->int_buf_size; j++)
                            {
                                sourceBuffers[0]->addToBuffer(pDevInt->interleaving_buffer + j*64, &cts, pDevInt->event_buffer+j, 1);
                                cts++;
                            }
                            // Update the 64-bit timestamp, take account of its wrap-around
                            pDevInt->buf_timestamp64 += tsdif;
                        }
                        else
                        {
                            // The interleaving buffer is empty
                            pDevInt->buf_timestamp64 = bts;
                        }
                        pDevInt->buf_timestamp = bts;
                        pDevInt->int_buf_size = datasize;
                        pDevInt->buf_timestamp_locked = true;
                        // Clear the interleaving buffer within the new packet's size
                        memset(pDevInt->interleaving_buffer, 0, sizeof(float)*datasize*64);
                        // Clear the event buffer as well
                        memset(pDevInt->event_buffer, 0, sizeof(uint64_t)*datasize);
                    }
                    // Convert data from ecube buffer into the interleaving buffer format
                    chid = chit->second; // Adjust the channel id to become the channel index
                    unsigned char* dp = ab->GetDataPointer();
                    const uint16_t* pData = (const uint16_t*)dp;
                    const char* pbits;
                    const uint32_t* pconvtbl;
                    switch (chid)
                    {
                        case 0:
                        case 3:
                            pbits = bits_port0;
                            pconvtbl = pDevInt->bit_conversion_tables;
                            break;
                        case 1:
                        case 4:
                            pbits = bits_port1;
                            pconvtbl = pDevInt->bit_conversion_tables+0x200;
                            break;
                        case 2:
                        case 5:
                        default:
                            pbits = bits_port2;
                            pconvtbl = pDevInt->bit_conversion_tables + 0x400;
                            break;
                    }
                    int bitchn_offset = chid >= 3 ? 32 : 0;
                    uint8_t dword_shift = chid>=3 ? 32 : 0;
                    for (unsigned long j = 0; j < datasize; j++)
                    {
                        uint16_t wrd = pData[j];

                        // Convert the word into packed 32-bit representation for this port
                        uint32_t packedwrd = pconvtbl[wrd & 0xFF] | pconvtbl[0x100 + (wrd >> 8)];
                        pDevInt->event_buffer[j] |= (uint64_t)packedwrd << dword_shift;

                        uint16_t msk = 1;
                        for (unsigned long k = 0; k < 16; k++)
                        {
                            int bitchn = pbits[k];
                            if (bitchn>=0)
                            {
                                float val = wrd&msk ? 5.0f : 0.0f; // Convert to 5V/0V values
                                pDevInt->interleaving_buffer[bitchn + bitchn_offset + 64*j] = val;
                            }
                            msk <<= 1;
                        }
                    }
                }
            }
        }
        if (pDevInt->data_format == EcubeDevInt::dfSeparateChannelsAnalog || pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
            ba = pDevInt->pStrmA->GetBuffersAcquired();
        else
            ba = pDevInt->pStrmD->GetBuffersAcquired();
    }
    return true;
}

bool EcubeThread::startAcquisition()
{
    pDevInt->buf_timestamp_locked = false;
    if (!isThreadRunning())
        startThread();

    if (!acquisition_running)
    {
        if (pDevInt->data_format == EcubeDevInt::dfSeparateChannelsAnalog || pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
            pDevInt->pStrmA->Start();
        else
            pDevInt->pStrmD->Start();
        acquisition_running = true;
    }

    return true;
}

bool EcubeThread::stopAcquisition()
{
    if (acquisition_running)
    {
        if (pDevInt->data_format == EcubeDevInt::dfSeparateChannelsAnalog || pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
            pDevInt->pStrmA->Stop();
        else
            pDevInt->pStrmD->Stop();
    }
    acquisition_running = false;
    return true;
}

void EcubeThread::run()
{
    // Call the base class's run
    DataThread::run();
}

void EcubeThread::setSpeakerVolume(double volume)
{
    pDevInt->pSpeaker->PutVolume(volume);
}

void EcubeThread::setSpeakerChannel(unsigned short channel)
{
    if (pDevInt->data_format == EcubeDevInt::dfSeparateChannelsAnalog)
        pDevInt->pStrmA->ConnectAudioMonitor(pDevInt->vpChannels.at(channel), pDevInt->pSpeakerChannel);
}


#endif