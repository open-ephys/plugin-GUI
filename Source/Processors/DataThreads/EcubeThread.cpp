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
#include "../SourceNode/SourceNode.h"
#include "../../UI/EcubeDialogComponent.h"
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
    unsigned n_channel_objects;
    std::map<int, int> chid_map;
    IEcubeAnalogAcquisitionPtr pStrmA;
    IEcubeDigitalInputStreamingPtr pStrmD;
    HeapBlock<float, true> interleaving_buffer;
    bool buf_timestamp_locked;
    unsigned long buf_timestamp;
    uint64 buf_timestamp64;
    unsigned long int_buf_size;
    DataFormat data_format;
};

static const char bits_port0[16] = { 23, 22, -1, 14, 11, -1, -1, 28, 12, 10, 27, 26, -1, -1, -1, -1 };
static const char bits_port1[16] = { 20, 21, 19, 18, 13, 6, 4, 5, 3, 2, -1, -1, 29, -1, 24, 25 };
static const char bits_port2[16] = { 16, 17, 15, 8, 9, 7, 0, 1, 31, 30, -1, -1, -1, -1, -1, -1 };



static std::vector<std::wstring> SafeArrayToVecStr(SAFEARRAY* sa)
{
    HRESULT hr;
    long lbound, ubound;

    if (SafeArrayGetElemsize(sa) != sizeof(BSTR*))
        _com_raise_error(E_FAIL);
    if(SafeArrayGetDim(sa)!=1)
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
        SAFEARRAY *sa = mp->GetChannels();
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
            SAFEARRAY *sa = pDevInt->pEcube->DetectNetworkDevices();
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
        pDevInt->n_channel_objects = 0;
        {
            String selmod = component.GetModuleName();
            if (selmod == "Headstage(s)")
            {
                m_samplerate = 25000.0f;
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
                        }
                    }
                }
                dataBuffer = new DataBuffer(pDevInt->n_channel_objects, 10000);
                // Create the interleaving buffer based on the number of channels
                pDevInt->interleaving_buffer.malloc(sizeof(float)* 1500 * pDevInt->n_channel_objects);
            }
            else if (selmod == "Panel Analog Input")
            {
                pDevInt->pModule = pDevInt->pDevice->OpenModule(_bstr_t(L"PanelAnalogInput"));
                m_samplerate = 25000.0f;// 40.0e6 / 572 original samplerate;
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
                dataBuffer = new DataBuffer(32, 10000);
                // The interleaving buffer is there just for short->float conversion
                pDevInt->interleaving_buffer.malloc(sizeof(float)* 1500);
            }
            else if (selmod == "Panel Digital Input")
            {
                pDevInt->pModule = pDevInt->pDevice->OpenModule(_bstr_t(L"PanelDigitalIO"));
                m_samplerate = 25000.0f;
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

                dataBuffer = new DataBuffer(64, 10000);
                // Create the interleaving buffer based on the number of digital ports
                pDevInt->interleaving_buffer.malloc(sizeof(float)* 1500 * 64);
            }
            else
                throw std::runtime_error("Invlid module selection");
        }

        pDevInt->buf_timestamp_locked = false;

        setDefaultChannelNamesAndType();

    }
    catch (_com_error& e)
    {
        // Convert COM errors to std::runtime_errors, to avoid making other files Windows-dependent
        throw std::runtime_error(std::string(e.Description()));
    }
}
void EcubeThread::getChannelsInfo(StringArray &Names_, Array<channelType> &type_, Array<int> &stream_, Array<int> &originalChannelNumber_, Array<float> &gains_)
{
    Names_ = Names;
    type_ = type;
    stream_ = stream;
    originalChannelNumber_ = originalChannelNumber;
    gains_ = gains;
}

/* This will give default names & gains to channels, unless they were manually modified by the user
In that case, the query channelModified, will return the values that need to be put */
void EcubeThread::setDefaultChannelNamesAndType()
{
    Names.clear();
    type.clear();
    stream.clear();
    gains.clear();
    originalChannelNumber.clear();
    String prefix;
    channelType common_type;

    int numch = getNumChannels();

    if (pDevInt->data_format == EcubeDevInt::dfSeparateChannelsAnalog)
    {
        prefix = "HS10_CH";
        common_type = DATA_CHANNEL;
    }
    else if (pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
    {
        prefix = "PAI";
        common_type = AUX_CHANNEL;
    }
    else //if (pDevInt->data_format == EcubeDevInt::dfDigital)
    {
        prefix = "PDI";
        common_type = AUX_CHANNEL;
    }

    if (numberingScheme != 1)
        prefix += "_";

    for (int i = 0; i < numch; i++)
    {
        Names.add(prefix + String(i));
        gains.add(getBitVolts());
        type.add(common_type);
        originalChannelNumber.add(i);
    }

    stream.add(0);
}

void EcubeThread::setDefaultNamingScheme(int scheme)
{
    numberingScheme = scheme;
    setDefaultChannelNamesAndType();
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

int EcubeThread::getNumChannels()
{
    if (pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
        return 32;
    else if (pDevInt->data_format == EcubeDevInt::dfDigital)
        return 64;
    else
        return pDevInt->n_channel_objects;
}

float EcubeThread::getSampleRate()
{
    return m_samplerate;
}

float EcubeThread::getBitVolts()
{
    return 10e3/32768; // For some reason the data is supposed to be in millivolts
}

bool EcubeThread::foundInputSource()
{
    return true;
}

bool EcubeThread::updateBuffer()
{
    unsigned long ba;
    int16 eventcode = 0;
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
                    if (!pDevInt->buf_timestamp_locked || (bts - pDevInt->buf_timestamp >= 3200 && pDevInt->buf_timestamp - bts >= 3200)
                        /*bts != pDevInt->buf_timestamp*/
                        || datasize != pDevInt->int_buf_size)
                    {
                        // The new buffer does not match interleaving buffer length, or has a different timestamp,
                        // or interleaving buffer is empty
                        if (pDevInt->buf_timestamp_locked)
                        {
                            // Interleaving buffer is not empty.
                            // Send its contents out to the application
                            int64 cts = pDevInt->buf_timestamp64 / 3200; // Convert eCube 80MHz timestamp into a 25kHz timestamp
                            for (unsigned long j = 0; j < pDevInt->int_buf_size; j++)
                            {
                                dataBuffer->addToBuffer(pDevInt->interleaving_buffer + j*nchan, &cts, &eventCode, 1);
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
                        pDevInt->interleaving_buffer[chid + nchan*j] = pData[j] * 10.0e3 / 32768; // OpenEphys uses 10e3 instead of just 10
                    }
                }
                else if (pDevInt->data_format == EcubeDevInt::dfInterleavedChannelsAnalog)
                {
                    if (pDevInt->buf_timestamp_locked)
                    {
                        // Update the 64-bit timestamp, take care of its wrap-around
                        unsigned tsdif = pDevInt->buf_timestamp - bts;
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
                        pDevInt->interleaving_buffer[j] = pData[j] * 10.0e3 / 32768;
                    }
                    unsigned long datasam = datasize / 32;
                    int64 cts = pDevInt->buf_timestamp64 / 3200; // Convert eCube's 80MHz timestamps into number of samples on the Panel Analog input (orig sample rate 1144)
                    for (unsigned long j = 0; j < datasam; j++)
                    {
                        dataBuffer->addToBuffer(pDevInt->interleaving_buffer+j*32, &cts, &eventCode, 1);
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
                            int64 cts = pDevInt->buf_timestamp64 / 3200; // Convert eCube 80MHz timestamp into a 25kHz timestamp
                            for (unsigned long j = 0; j < pDevInt->int_buf_size; j++)
                            {
                                dataBuffer->addToBuffer(pDevInt->interleaving_buffer + j*64, &cts, &eventCode, 1);
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
                    }
                    // Convert data from ecube buffer into the interleaving buffer format
                    chid = chit->second; // Adjust the channel id to become the channel index
                    unsigned char* dp = ab->GetDataPointer();
                    const uint16_t* pData = (const uint16_t*)dp;
                    const char* pbits;
                    switch (chid)
                    {
                    case 0:
                    case 3:
                        pbits = bits_port0;
                        break;
                    case 1:
                    case 4:
                        pbits = bits_port1;
                        break;
                    case 2:
                    case 5:
                    default:
                        pbits = bits_port2;
                        break;
                    }
                    int bitchn_offset = chid >= 3 ? 32 : 0;
                    for (unsigned long j = 0; j < datasize; j++)
                    {
                        uint16_t wrd = pData[j];
                        uint16_t msk = 1;
                        for (unsigned long k = 0; k < 16; k++)
                        {
                            int bitchn = pbits[k];
                            if (bitchn>=0)
                            {
                                float val = wrd&msk ? 1.0e3f : 0.0f;
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


#endif