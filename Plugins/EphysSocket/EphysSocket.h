#ifndef __EPHYSSOCKETH__
#define __EPHYSSOCKETH__

#include <DataThreadHeaders.h>

namespace EphysSocketNode
{
    class EphysSocket : public DataThread, public Timer
    {

    public:
        EphysSocket(SourceNode* sn);
        ~EphysSocket();

        // Interface fulfillment
        bool foundInputSource() override;
        int getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessor) const override;
        int getNumTTLOutputs(int subprocessor) const override;
        float getSampleRate(int subprocessor) const override;
        float getBitVolts(const DataChannel* chan) const override;
        int getNumChannels() const;

        // User defined
        int port = 5000;
        float sample_rate = 30e3;
        float data_scale = 0.195;
        uint16_t data_offset = 32768;
        bool transpose = true;
        int num_samp = 250;
        int num_channels = 64;

        void resizeChanSamp();
        void tryToConnect();

        GenericEditor* createEditor(SourceNode* sn);
        static DataThread* createDataThread(SourceNode* sn);

    private:

        bool updateBuffer() override;
        bool startAcquisition() override;
        bool stopAcquisition()  override;
        void timerCallback() override;

        bool connected = false;

       ScopedPointer<DatagramSocket> socket;

        uint16_t *recvbuf;
        float *convbuf;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EphysSocket);
    };
}
#endif