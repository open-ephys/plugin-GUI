#ifndef FAKESOURCENODE_H
#define FAKESOURCENODE_H

#include <ProcessorHeaders.h>
#include <NonAPIHeaders.h>

/** Collection of optional, settable parameters for configuring the FakeSourceNode for testing. */
struct FakeSourceNodeParams {
    int channels = 1;
    float sampleRate = 20000.0f;
    float bitVolts = 1.0f;
    int streams = 1;
    uint32_t metadata_size_bytes = 0;
};

class TESTABLE FakeSourceNode : public GenericProcessor {
public:
    explicit FakeSourceNode(const FakeSourceNodeParams &params);
    void updateSettings() override;
    void process(AudioBuffer<float>& continuousBuffer) override;
    void setParams(const FakeSourceNodeParams &params);
private:
    FakeSourceNodeParams params_;
    OwnedArray<DataStream> cached_datastreams_;
};


#endif
