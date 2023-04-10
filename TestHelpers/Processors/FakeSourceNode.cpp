#include "FakeSourceNode.h"

FakeSourceNode::FakeSourceNode() : GenericProcessor("Fake Source Node")
{
    
}

void FakeSourceNode::addMessageChannel() {
    
}

void FakeSourceNode::clearStreams()
{
    dataStreams.clear();
    continuousChannels.clear();
    eventChannels.clear();
}

void FakeSourceNode::addTestDataStream(int numChannels, float sampleRate) {
    
    int streamIndex = dataStreams.size();
    
    LOGD("Adding stream at index ", streamIndex);
    
    DataStream::Settings settings
    {
        "FakeSourceNodeStream" + String(streamIndex+1),
        "description",
        "identifier",
        sampleRate
    };
    
    dataStreams.add(new DataStream(settings));
    
    for (int channelIndex = 0; channelIndex < numChannels; channelIndex++)
    {
        
        ContinuousChannel::Settings settings{
            ContinuousChannel::Type::ELECTRODE,
            "CH" + String(channelIndex + 1),
            String(channelIndex + 1),
            "identifier",
            
            1.0, // bitVolts
            
            dataStreams.getLast()
        };
        
        continuousChannels.add(new ContinuousChannel(settings));
    }
    
    {
        EventChannel::Settings settings{
            EventChannel::Type::TTL,
            "EventChannel",
            "description",
            "identifier",
            
            dataStreams.getLast()
        };
        
        eventChannels.add(new EventChannel(settings));
    }

}
