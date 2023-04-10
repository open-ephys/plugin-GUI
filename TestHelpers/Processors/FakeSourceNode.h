#ifndef FAKESOURCENODE_H
#define FAKESOURCENODE_H

#include <ProcessorHeaders.h>
#include <NonAPIHeaders.h>

/**
 
    Creates a source node for testing downstream processors
 
 */
class TESTABLE FakeSourceNode : public GenericProcessor
{
public:
    
    /** Constructor */
    FakeSourceNode();
    
    /** Adds the message channel */
    void addMessageChannel();
    
    /** Clears stream settings */
    void clearStreams();
    
    /** Adds a stream for testing  */
    void addTestDataStream(int numChannels, float sampleRate);
    
    /** Empty implementation of process method (only pure virtual method) */
    void process(AudioBuffer<float>& continuousBuffer) override { }
    
};


#endif
