#include "FakeSourceNode.h"

FakeSourceNode::FakeSourceNode(const FakeSourceNodeParams &params)
    : GenericProcessor("Fake Source Node", true),
      params_(params) {
    setProcessorType(Plugin::Processor::Type::SOURCE);
}

void FakeSourceNode::updateSettings() {
    DataStream::Settings settings
    {
        "FakeSourceNode",
        "description",
        "identifier",
        params_.sampleRate
    };
    
    dataStreams.add(new DataStream(settings));
    
    for (int index= 0; index < params_.channels; index++)
    {
        
        ContinuousChannel::Settings settings{
            ContinuousChannel::Type::ELECTRODE,
            "CH" + String(index),
            String(index),
            "identifier",
            params_.bitVolts,
            dataStreams.getFirst()
        };
        
        continuousChannels.add(new ContinuousChannel(settings));
    }

}

void FakeSourceNode::process(AudioBuffer<float>& continuousBuffer) {}

