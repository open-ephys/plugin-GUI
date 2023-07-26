#include <stdio.h>

#include "gtest/gtest.h"

#include "../LfpDisplayNode.h"
#include "../LfpDisplayCanvas.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>



/*
    Class that can hold buffers of sample data written to the canvas and produce an 'expected' result image
     
    Used to verify the LFP Canvas' ability to accurately display voltages
 */
class ExpectedImage {
public:
    ExpectedImage(int channels, int samples) : numChannels(channels), numSamplesInView(samples), lastSampleWritten(0){
        for(int c = 0; c < numChannels; c++) {
            Array<float> newBuffer;
            newBuffer.insertMultiple(0, 0, samples);
            buffers.add(newBuffer);
        }
    }

    /*Adds samples from an AudioBuffer to internal buffer. Should be same buffer passed to the LFP Canvas*/
    void addToBuffer(AudioBuffer<float> newSamples) {
        for(int channelIndex = 0; channelIndex < numChannels; channelIndex++) {
            const float* channelReadPointer = newSamples.getReadPointer(channelIndex);
            int currentSampleBufferIndex = lastSampleWritten;
            for(int sampleIndex = 0; sampleIndex < newSamples.getNumSamples(); sampleIndex++) {
                float val = channelReadPointer[sampleIndex];
                Array<float>& writeBuffer = buffers.getReference(channelIndex);
                writeBuffer.set(currentSampleBufferIndex++, val);
                if(currentSampleBufferIndex >= numSamplesInView) {
                    currentSampleBufferIndex = 0;
                }
            }

        }
        lastSampleWritten += newSamples.getNumSamples() % numSamplesInView;
    }
    
    /*Constructs the trace information from internal buffers into an Image object*/
    Image getImage(int width, int height, Array<Colour> channelColors, Colour backgroundColor, Colour midlineColor, int scaleFactor) {
        Image expectedImage(Image::ARGB, width,height, true);
        
        //Fill image with background color
        Graphics g(expectedImage);
        g.fillAll(backgroundColor);
        int heightPerChannel = height/numChannels;
        float samplesPerPixel = (float)numSamplesInView/(float)width;
        
        //Draw Midline
        for(int channelIndex = 0; channelIndex < numChannels; channelIndex += 1) {
            for(int x = 0; x < width; x++) {
                expectedImage.setPixelAt(x, channelIndex * heightPerChannel + heightPerChannel/2, midlineColor);
            }
        }

        //Draw trace data
        for(int channelIndex = 0; channelIndex < numChannels; channelIndex++) {
            int lastPixelY = 0;
            for(int xWritePixel = 0; xWritePixel < width; xWritePixel++) {
                int avg = 0;
                int startSampleIndex = std::ceil(xWritePixel * samplesPerPixel);
                int endSampleIndex = (std::ceil((xWritePixel + 1) * samplesPerPixel) - 1);
                for(int i = startSampleIndex; i < endSampleIndex; i++) {
                    avg += buffers[channelIndex][i];
                }
                
                avg /= (endSampleIndex - startSampleIndex);
                

                int amplitude = float(avg)/float(scaleFactor) * ((height/numChannels)/2);
                int channelMidline = channelIndex * heightPerChannel + heightPerChannel/2;
                int yWritePixel = channelMidline - amplitude;
                if(yWritePixel >= 0 && yWritePixel < height){
                    expectedImage.setPixelAt(xWritePixel, yWritePixel, channelColors[channelIndex]);
                    if(xWritePixel != 0) {
                        //Need to fill in gaps if  difference between y pixels
                        int currentY = lastPixelY;
                        while(currentY != yWritePixel) {
                            expectedImage.setPixelAt(xWritePixel, currentY, channelColors[channelIndex]);
                            currentY = yWritePixel > currentY ? currentY + 1 : currentY - 1;
                        }
                    }
                    lastPixelY = yWritePixel;
                }
            }
        }
        
        //Draw playback line
        int playbackXPixel = int(std::ceil(float(lastSampleWritten)/float(samplesPerPixel))) % width + 1;
        for(int playbackYPixel = 0; playbackYPixel < height; playbackYPixel+=2) {
            expectedImage.setPixelAt(playbackXPixel, playbackYPixel, backgroundColor);
            expectedImage.setPixelAt(playbackXPixel, playbackYPixel + 1, Colours::yellow);
        }

        
        return expectedImage;
    }
    
    int lastSampleWritten;
    
    int numSamplesInView;
    int numChannels;
    Array<Array<float>> buffers;
    
    int height;
    int width;
    
};


class LfpDisplayNodeTests : public ::testing::Test {
protected:
    void SetUp() override {
        num_channels = 16;
        tester = std::make_unique<ProcessorTester>(FakeSourceNodeParams{
            num_channels,
            sample_rate_,
            bitVolts_
        });

        processor = tester->Create<LfpViewer::LfpDisplayNode>(Plugin::Processor::SINK);
        
        width = 0;
        height = 0;
        x = 0;
        y = 0;
        
        midlineColour = Colour(50,50,50);
        playheadColour = Colours::yellow;
    }

    void TearDown() override {

    }
    
    
    /*Create a new AudioBuffer filled with step data*/
    AudioBuffer<float> CreateBuffer(float starting_value, float step, int num_channels, int num_samples) {
        AudioBuffer<float> input_buffer(num_channels, num_samples);

        // in microvolts
        float cur_value = starting_value;
        for (int chidx = 0; chidx < num_channels; chidx++) {
            for (int sample_idx = 0; sample_idx < num_samples; sample_idx++) {
                input_buffer.setSample(chidx, sample_idx, cur_value);
                cur_value += step;
            }
        }
        return input_buffer;
    }
    
    /*Creates a new AudioBuffer filled with sinusoidal waves*/
    AudioBuffer<float> CreateBufferSinusoidal(int cycles, int num_channels, int num_samples, int amplitude) {
        AudioBuffer<float> input_buffer(num_channels, num_samples);
        float pi = 3.1415;
        // in microvolts
        for (int chidx = 0; chidx < num_channels; chidx++) {
            for (int sample_idx = 0; sample_idx < num_samples; sample_idx++) {
                float value = amplitude * sin(float(sample_idx)/(float(num_samples)/float(cycles)) * 2 * pi);
                input_buffer.setSample(chidx, sample_idx, value);
            }
        }
        return input_buffer;
    }
    
    /*Writes data from an AudioBuffer to processor. Verifies that the AudioBuffer is not changed*/
    void WriteBlock(AudioBuffer<float> &buffer) {
        auto audio_processor = (AudioProcessor *)processor;
        auto data_streams = processor->getDataStreams();
        ASSERT_EQ(data_streams.size(), 1);
        auto streamId = data_streams[0]->getStreamId();
        processor -> setParameter(0, streamId);
        HeapBlock<char> data;
        size_t dataSize = SystemEvent::fillTimestampAndSamplesData(
            data,
            processor,
            streamId,
            current_sample_index,
            0,
            buffer.getNumSamples(),
            0);
        MidiBuffer eventBuffer;
        eventBuffer.addEvent(data, dataSize, 0);

        auto original_buffer = buffer;
        audio_processor->processBlock(buffer, eventBuffer);
        // Assert the buffer hasn't changed after process()
        ASSERT_EQ(buffer.getNumSamples(), original_buffer.getNumSamples());
        ASSERT_EQ(buffer.getNumChannels(), original_buffer.getNumChannels());
        for (int chidx = 0; chidx < buffer.getNumChannels(); chidx++) {
            for (int sample_idx = 0; sample_idx < buffer.getNumSamples(); ++sample_idx) {
                ASSERT_EQ(buffer.getSample(chidx, sample_idx), original_buffer.getSample(chidx, sample_idx));
            }
        }
        current_sample_index += buffer.getNumSamples();
    }
    
    /*Gets information from canvas needed to build an ExpectedImage*/
    void SetExpectedImageParameters(LfpViewer::LfpDisplayCanvas* canvas) {
        canvas -> getChannelBitmapBounds(0, x, y, width, height);
        canvas -> getChannelColors(0, channelColors, backgroundColour);
        ASSERT_EQ(channelColors.size(), num_channels);
    }
    

    /*Compares 2 Image objects and returns the number of different pixels*/
    int GetImageDifferencePixelCount(Image expectedImage, Image actualImage) {
        int missCount = 0;
        Image diffImage(Image::ARGB, width, height, true);
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                Colour expectedPixel = expectedImage.getPixelAt(x, y);
                Colour actualPixel = actualImage.getPixelAt(x, y);
                if(expectedPixel != actualPixel) {
                    //Allow some variance - if expectedPixel is a voltage Pixel then see if actualPixel +- 1 pixel is also a voltage Pixel.
                    //If true then don't count as a miss
                    if(expectedPixel != backgroundColour && expectedPixel != midlineColour && expectedPixel != playheadColour) {
                        Colour postiveVariance = actualImage.getPixelAt(x, y + 1);
                        Colour negativeVariance = actualImage.getPixelAt(x, y - 1);
                        if(postiveVariance == expectedPixel || negativeVariance == expectedPixel) {
                            continue;
                        }
                    }
                    //Same for if actualPixel is a voltagePixel and expectedPixel isn't
                    if(actualPixel != backgroundColour && actualPixel != midlineColour && actualPixel != playheadColour) {
                        Colour postiveVariance = expectedImage.getPixelAt(x, y + 1);
                        Colour negativeVariance = expectedImage.getPixelAt(x, y - 1);
                        if(postiveVariance == actualPixel || negativeVariance == actualPixel) {
                            continue;
                        }
                    }
                    missCount += 1;
                }
            }
        }
        return missCount;
    }
    
    void DumpPng(String path, Image image) {
        FileOutputStream stream ((File (path)));
        PNGImageFormat pngWriter;
        pngWriter.writeImageToStream(image, stream);
    }

    LfpViewer::LfpDisplayNode *processor;
    
    int num_channels;
    float bitVolts_ = 1.0;
    std::unique_ptr<ProcessorTester> tester;
    int64_t current_sample_index = 0;
    float sample_rate_ = 2000;
    
    Image expectedImage;
    Colour midlineColour;
    Colour backgroundColour;
    Colour playheadColour;

    Array<Colour> channelColors;
    
    
    int width;
    int height;
    int x;
    int y;
};


TEST_F(LfpDisplayNodeTests, VisualIntegrityTest) {
    const int canvasX = 600;
    const int canvasY = 800;

    //Initialize LFP Canvas
    std::unique_ptr<LfpViewer::LfpDisplayCanvas> canvas = std::make_unique<LfpViewer::LfpDisplayCanvas>(processor, LfpViewer::SplitLayouts::SINGLE, false);
    canvas -> update();
    canvas -> setSize(canvasX, canvasY);
    canvas -> resized();
    canvas -> setVisible(true);


    //Get LFP size parameters from canvas
    SetExpectedImageParameters(canvas.get());
    
    //Create snapshot of canvas channel bitmap and expected image
    Rectangle<int> canvasSnapshot(x, y, width, height);
    ExpectedImage expected(num_channels, sample_rate_);
    
    
    tester->startAcquisition(false);
    canvas->beginAnimation();

    //Add 5 10Hz waves with +-125uV amplitude
    auto input_buffer = CreateBufferSinusoidal(5, num_channels, 1000, 125);
    WriteBlock(input_buffer);
    expected.addToBuffer(input_buffer);
    canvas -> refreshState();
    Image canvas_image = canvas -> createComponentSnapshot(canvasSnapshot);
    Image expected_image = expected.getImage(width, height, channelColors, backgroundColour, midlineColour, 125);
    int missCount = GetImageDifferencePixelCount(expected_image, canvas_image);
    ASSERT_LE(missCount/(width * height), 0.01f);
    
    //Add 5 10Hz waves with +-250uV amplitude
    input_buffer = CreateBufferSinusoidal(5, num_channels, 1000, 250);
    WriteBlock(input_buffer);
    expected.addToBuffer(input_buffer);
    canvas -> refreshState();
    
    canvas_image = canvas -> createComponentSnapshot(canvasSnapshot);
    expected_image = expected.getImage(width, height, channelColors, backgroundColour, midlineColour, 125);
    missCount = GetImageDifferencePixelCount(expected_image, canvas_image);
    ASSERT_LE(float(missCount)/float(width * height), 0.01f);
    
    
    //Add 10 40Hz waves with +-250uV amplitude
    input_buffer = CreateBufferSinusoidal(10, num_channels, 500, 250);
    WriteBlock(input_buffer);
    expected.addToBuffer(input_buffer);
    canvas -> refreshState();
    
    canvas_image = canvas -> createComponentSnapshot(canvasSnapshot);
    expected_image = expected.getImage(width, height, channelColors, backgroundColour, midlineColour, 125);
    missCount = GetImageDifferencePixelCount(expected_image, canvas_image);
    ASSERT_LE(float(missCount)/float(width * height), 0.01f);
    
    //Resize canvas to have half the vertical height and twice the uV range
    canvas -> setChannelHeight(0, 20);
    canvas -> setChannelRange(0, 500, ContinuousChannel::Type::ELECTRODE);
    canvas -> refreshState();
    SetExpectedImageParameters(canvas.get());

    canvasSnapshot.setBounds(x, y, width, height);
    
    canvas_image = canvas -> createComponentSnapshot(canvasSnapshot);
    expected_image = expected.getImage(width, height, channelColors, backgroundColour, midlineColour, 250);
    missCount = GetImageDifferencePixelCount(expected_image, canvas_image);
    ASSERT_LE(float(missCount)/float(width * height), 0.01f);

    tester->stopAcquisition();
    

    

}

TEST_F(LfpDisplayNodeTests, DataIntegrityTest) {
    int num_samples = 100;
    tester->startAcquisition(false);

    auto input_buffer = CreateBuffer(1000.0, 20.0, num_channels, num_samples);
    WriteBlock(input_buffer);

    tester->stopAcquisition();

}



