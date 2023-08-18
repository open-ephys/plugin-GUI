#include "FakeSourceNode.h"

FakeSourceNode::FakeSourceNode(const FakeSourceNodeParams &params)
    : GenericProcessor("Fake Source Node", true),
      params_(params) {
    setProcessorType(Plugin::Processor::Type::SOURCE);
}

void FakeSourceNode::updateSettings() {
    // Don't recreate datastreams, or the IDs / pointer locations keep changing.
    if (cached_datastreams_.size() == 0) {
        for(int i = 0; i < params_.streams; i++){
            DataStream::Settings settings
                {
                    "FakeSourceNode"+String(i),
                    "description",
                    "identifier",
                    params_.sampleRate
                };

            cached_datastreams_.add(new DataStream(settings));
        }
    }

    continuousChannels.clear();
    dataStreams.clear();
    for (const auto &item : cached_datastreams_) {
        // Copy it over
        dataStreams.add(new DataStream(*item));
    }
    for(int i = 0; i < params_.streams; i++){
        for (int index = 0; index < params_.channels; index++) {
            ContinuousChannel::Settings settings{
                ContinuousChannel::Type::ELECTRODE,
                "CH" + String(index),
                String(index),
                "identifier",
                params_.bitVolts,
                dataStreams[i]
                
            };
            
            continuousChannels.add(new ContinuousChannel(settings));
        }
    }

    EventChannel::Settings settings{
        EventChannel::Type::TTL,
        "TTL",
        "TTL",
        "identifier.ttl.events",
        dataStreams.getFirst(),
    };
    eventChannels.add(new EventChannel(settings));
    eventChannels.getFirst()->setIdentifier("sourceevent");
    eventChannels.getFirst()->addProcessor(processorInfo.get());
}

void FakeSourceNode::setParams(const FakeSourceNodeParams &params) {
    params_ = params;
    cached_datastreams_.clear();
}



void FakeSourceNode::process(AudioBuffer<float>& continuousBuffer) {}

