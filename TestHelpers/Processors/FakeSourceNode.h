#ifndef FAKESOURCENODE_H
#define FAKESOURCENODE_H

#include <ProcessorHeaders.h>
#include <NonAPIHeaders.h>

class TESTABLE FakeSourceNode : public GenericProcessor {
public:
    FakeSourceNode(int channels = 1, float sampleRate = 20000.0f);
    
    void addMessageChannel();
    void addTestDataStreams();
    
    void process(AudioBuffer<float>& continuousBuffer) override;
    
private:
    int channels;
    float sampleRate;
};


#endif
