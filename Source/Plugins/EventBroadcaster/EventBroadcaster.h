/*
  ==============================================================================

    EventBroadcaster.h
    Created: 22 May 2015 3:31:50pm
    Author:  Christopher Stawarz

  ==============================================================================
*/

#ifndef EVENTBROADCASTER_H_INCLUDED
#define EVENTBROADCASTER_H_INCLUDED

#include <ProcessorHeaders.h>

#ifdef ZEROMQ
    #ifdef WIN32
        #include <zmq.h>
        #include <zmq_utils.h>
    #else
        #include <zmq.h>
    #endif
#endif

class EventBroadcaster : public GenericProcessor
{
public:
    // ids for format combobox
    enum Format { RAW_BINARY = 1, HEADER_ONLY, HEADER_AND_JSON };

    EventBroadcaster();

    AudioProcessorEditor* createEditor() override;

    int getListeningPort() const;
    // returns 0 on success, else the errno value for the error that occurred.
    int setListeningPort(int port, bool forceRestart = false);

    int getOutputFormat() const;
    void setOutputFormat(int format);

    void process(AudioSampleBuffer& continuousBuffer) override;
    void handleEvent(const EventChannel* channelInfo, const MidiMessage& event, int samplePosition = 0) override;
    void handleSpike(const SpikeChannel* channelInfo, const MidiMessage& event, int samplePosition = 0) override;

    void saveCustomParametersToXml(XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;

private:
    struct MsgPart
    {
        String name;
        MemoryBlock data;
    };

    class ZMQContext : public ReferenceCountedObject
    {
    public:
        ZMQContext();
        ~ZMQContext();
        void* createZMQSocket();
    private:
        void* context;
    };

    class ZMQSocket
    {
    public:
        ZMQSocket();
        ~ZMQSocket();

        bool isValid() const;
        int getBoundPort() const;

        int send(const void* buf, size_t len, int flags);
        int bind(int port);
        int unbind();
    private:
        int boundPort;
        void* socket;

        // see here for why the context can't just be static:
        // https://github.com/zeromq/libzmq/issues/1708
        SharedResourcePointer<ZMQContext> context;
    };

    void sendEvent(const InfoObjectCommon* channel, const MidiMessage& msg) const;

    int sendMessage(const Array<MsgPart>& parts) const;

    // add metadata from an event to a DynamicObject
    static void populateMetaData(const MetaDataEventObject* channel,
        const EventBasePtr event, DynamicObject::Ptr dest);

    static String getEndpoint(int port);

    // called from setListeningPort() depending on success/failure of ZMQ operations
    void reportActualListeningPort(int port);

    // share a "dumb" pointer that doesn't take part in reference counting.
    // want the context to be terminated by the time the static members are
    // destroyed (see: https://github.com/zeromq/libzmq/issues/1708)
    static ZMQContext* sharedContext;
    static CriticalSection sharedContextLock;
    ScopedPointer<ZMQSocket> zmqSocket;
    int listeningPort;

    int outputFormat;

    // ---- utilities for formatting binary data and metadata ----

    // a fuction to convert metadata or binary data to a form we can add to the JSON object
    typedef var(*DataToVarFcn)(const void* value, unsigned int dataLength);

    template <typename T>
    static var binaryValueToVar(const void* value, unsigned int dataLength);

    static var stringValueToVar(const void* value, unsigned int dataLength);

    static DataToVarFcn getDataReader(BaseType dataType);
};


#endif  // EVENTBROADCASTER_H_INCLUDED
