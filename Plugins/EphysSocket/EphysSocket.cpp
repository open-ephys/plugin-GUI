#ifdef _WIN32
#include <Windows.h>
#endif

#include "EphysSocket.h"
#include "EphysSocketEditor.h"

using namespace EphysSocketNode;

DataThread* EphysSocket::createDataThread(SourceNode *sn)
{
    return new EphysSocket(sn);
}

EphysSocket::EphysSocket(SourceNode* sn) : DataThread(sn)
{
    socket = new DatagramSocket();
    socket->bindToPort(port);
    connected = (socket->waitUntilReady(true, 1000) == 1); // Try to automatically open, dont worry if it does not work
    sourceBuffers.add(new DataBuffer(num_channels, num_channels * num_samp * 4 * 5)); // start with 2 channels and automatically resize
    recvbuf = (uint16_t *)malloc(num_channels * num_samp * 2);
    convbuf = (float *)malloc(num_channels * num_samp * 4);
}

GenericEditor* EphysSocket::createEditor(SourceNode* sn)
{
    return new EphysSocketEditor(sn, this);
}

void EphysSocket::timerCallback()
{
    stopTimer();
}

EphysSocket::~EphysSocket()
{
    free(recvbuf);
    free(convbuf);
}

void EphysSocket::resizeChanSamp()
{
    sourceBuffers[0]->resize(num_channels, num_channels * num_samp * 4 * 5);
    recvbuf = (uint16_t *)realloc(recvbuf, num_channels * num_samp * 2);
    convbuf = (float *)realloc(convbuf, num_channels * num_samp * 4);
    //timestamps.resize(num_samp);
    //ttlEventWords.resize(num_samp);
}

int EphysSocket::getNumChannels() const
{
    return num_channels;
}

int EphysSocket::getNumDataOutputs(DataChannel::DataChannelTypes type, int subproc) const
{
    if (type == DataChannel::HEADSTAGE_CHANNEL)
        return num_channels;
    else
        return 0; 
}

int EphysSocket::getNumTTLOutputs(int subproc) const
{
    return 0; 
}

float EphysSocket::getSampleRate(int subproc) const
{
    return sample_rate;
}

float EphysSocket::getBitVolts (const DataChannel* ch) const
{
    return 0.195f;
}

bool EphysSocket::foundInputSource()
{
    return connected;
}

bool EphysSocket::startAcquisition()
{
    startThread();
    return true;
}

void  EphysSocket::tryToConnect()
{
    socket->shutdown();
    socket = new DatagramSocket();
    socket->bindToPort(port);
    connected = (socket->waitUntilReady(true, 1000) == 1);
}

bool EphysSocket::stopAcquisition()
{
    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }

    waitForThreadToExit(500);

    sourceBuffers[0]->clear();
    return true;
}

bool EphysSocket::updateBuffer()
{
    int rc = socket->read(recvbuf, num_channels * num_samp * 2, true);

    if (rc == -1) return false;

    // Transpose because the chunkSize arguement in addToBuffer does not seem to do anything
    if (transpose) {
        int k = 0;
        for (int i = 0; i < num_samp; i++) {
            for (int j = 0; j < num_channels; j++) {
                convbuf[k++] = 0.195 *  (float)(recvbuf[j*num_samp + i] - 32768);
            }
        }
    } else {
        for (int i = 0; i < num_samp * num_channels; i++)
            convbuf[i] = 0.195 *  (float)(recvbuf[i] - 32768);
    }

    sourceBuffers[0]->addToBuffer(convbuf, &timestamps.getReference(0), &ttlEventWords.getReference(0), num_samp, 1);

    return true;
}