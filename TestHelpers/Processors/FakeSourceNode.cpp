#include "FakeSourceNode.h"

FakeSourceNode::FakeSourceNode(int channels, float sampleRate) : GenericProcessor("Fake Source Node"), channels(channels), sampleRate(sampleRate){}

void FakeSourceNode::addMessageChannel() {
    
}

void FakeSourceNode::addTestDataStreams() {
    DataStream::Settings settings
    {
        "FakeSourceNode",
        "description",
        "identifier",
        sampleRate
    };
    
    dataStreams.add(new DataStream(settings));
    
    for (int index= 0; index < channels; index++)
    {
        
        ContinuousChannel::Settings settings{
            ContinuousChannel::Type::ELECTRODE,
            "CH" + String(index),
            String(index),
            "identifier",
            
            1.0,
            
            dataStreams.getFirst()
        };
        
        continuousChannels.add(new ContinuousChannel(settings));
    }

}

void FakeSourceNode::process(AudioBuffer<float>& continuousBuffer) {}

