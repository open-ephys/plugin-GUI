/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>

#include "gtest/gtest.h"

#include "../LfpDisplayCanvas.h"
#include "../LfpDisplayNode.h"
#include <ModelApplication.h>
#include <ModelProcessors.h>
#include <ProcessorHeaders.h>
#include <TestFixtures.h>

/*
    Class that can hold buffers of sample data written to the canvas and produce an 'expected' result image
     
    Used to verify the LFP Canvas' ability to accurately display voltages
 */
class ExpectedImage
{
public:
    ExpectedImage (int channels, int samples) : numChannels (channels), numSamplesInView (samples), lastSampleWritten (0)
    {
        for (int c = 0; c < numChannels; c++)
        {
            Array<float> newBuffer;
            newBuffer.insertMultiple (0, 0, samples);
            buffers.add (newBuffer);
        }
    }

    /*Adds samples from an AudioBuffer to internal buffer. Should be same buffer passed to the LFP Canvas*/
    void addToBuffer (AudioBuffer<float> newSamples)
    {
        for (int channelIndex = 0; channelIndex < numChannels; channelIndex++)
        {
            const float* channelReadPointer = newSamples.getReadPointer (channelIndex);
            int currentSampleBufferIndex = lastSampleWritten;
            for (int sampleIndex = 0; sampleIndex < newSamples.getNumSamples(); sampleIndex++)
            {
                float val = channelReadPointer[sampleIndex];
                Array<float>& writeBuffer = buffers.getReference (channelIndex);
                writeBuffer.set (currentSampleBufferIndex++, val);
                if (currentSampleBufferIndex >= numSamplesInView)
                {
                    currentSampleBufferIndex = 0;
                }
            }
        }
        lastSampleWritten += newSamples.getNumSamples() % numSamplesInView;
    }

    /*Constructs the trace information from internal buffers into an Image object*/
    Image getImage (int width, int height, Array<Colour> channelColours, Colour backgroundColour, Colour midlineColour, int scaleFactor) const
    {
        Image expectedImage (Image::ARGB, width, height, true, SoftwareImageType());

        //Fill image with background colour
        Graphics g (expectedImage);
        g.fillAll (backgroundColour);
        int heightPerChannel = height / numChannels;
        float samplesPerPixel = (float) numSamplesInView / (float) width;

        //Draw Midline
        for (int channelIndex = 0; channelIndex < numChannels; channelIndex += 1)
        {
            for (int x = 0; x < width; x++)
            {
                expectedImage.setPixelAt (x, channelIndex * heightPerChannel + heightPerChannel / 2, midlineColour);
            }
        }

        //Draw trace data
        for (int channelIndex = 0; channelIndex < numChannels; channelIndex++)
        {
            int lastPixelY = 0;
            for (int xWritePixel = 0; xWritePixel < width; xWritePixel++)
            {
                int avg = 0;
                int startSampleIndex = std::ceil (xWritePixel * samplesPerPixel);
                int endSampleIndex = (std::ceil ((xWritePixel + 1) * samplesPerPixel) - 1);
                for (int i = startSampleIndex; i < endSampleIndex; i++)
                {
                    avg += buffers[channelIndex][i];
                }

                avg /= (endSampleIndex - startSampleIndex);

                int amplitude = float (avg) / float (scaleFactor) * ((height / numChannels) / 2);
                int channelMidline = channelIndex * heightPerChannel + heightPerChannel / 2;
                int yWritePixel = channelMidline - amplitude;
                if (yWritePixel >= 0 && yWritePixel < height)
                {
                    expectedImage.setPixelAt (xWritePixel, yWritePixel, channelColours[channelIndex]);
                    if (xWritePixel != 0)
                    {
                        //Need to fill in gaps if  difference between y pixels
                        int currentY = lastPixelY;
                        while (currentY != yWritePixel)
                        {
                            expectedImage.setPixelAt (xWritePixel, currentY, channelColours[channelIndex]);
                            currentY = yWritePixel > currentY ? currentY + 1 : currentY - 1;
                        }
                    }
                    lastPixelY = yWritePixel;
                }
            }
        }

        //Draw playback line
        int playbackXPixel = int (std::ceil (float (lastSampleWritten) / float (samplesPerPixel))) % width + 1;
        for (int playbackYPixel = 0; playbackYPixel < height; playbackYPixel += 2)
        {
            expectedImage.setPixelAt (playbackXPixel, playbackYPixel + 1, Colours::yellow);
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

class LfpDisplayNodeTests : public testing::Test
{
protected:
    void SetUp() override
    {
        numChannels = 16;
        tester = std::make_unique<ProcessorTester> (TestSourceNodeBuilder (FakeSourceNodeParams {
            numChannels,
            sampleRate,
            bitVolts }));

        processor = tester->createProcessor<LfpViewer::LfpDisplayNode> (Plugin::Processor::SINK);

        width = 0;
        height = 0;
        x = 0;
        y = 0;

        midlineColour = Colour (50, 50, 50);
        playheadColour = Colours::yellow;
    }

    /*Create a new AudioBuffer filled with step data*/
    AudioBuffer<float> createBuffer (float starting_value, float step, int numChannels, int numSamples)
    {
        AudioBuffer<float> inputBuffer (numChannels, numSamples);

        // in microvolts
        float currValue = starting_value;
        for (int chidx = 0; chidx < numChannels; chidx++)
        {
            for (int sampleIdx = 0; sampleIdx < numSamples; sampleIdx++)
            {
                inputBuffer.setSample (chidx, sampleIdx, currValue);
                currValue += step;
            }
        }
        return inputBuffer;
    }

    /*Creates a new AudioBuffer filled with sinusoidal waves*/
    AudioBuffer<float> createBufferSinusoidal (int cycles, int numChannels, int numSamples, int amplitude)
    {
        AudioBuffer<float> inputBuffer (numChannels, numSamples);
        float pi = 3.1415;
        // in microvolts
        for (int chidx = 0; chidx < numChannels; chidx++)
        {
            for (int sampleIdx = 0; sampleIdx < numSamples; sampleIdx++)
            {
                float value = amplitude * sin (float (sampleIdx) / (float (numSamples) / float (cycles)) * 2 * pi);
                inputBuffer.setSample (chidx, sampleIdx, value);
            }
        }
        return inputBuffer;
    }

    /*Writes data from an AudioBuffer to processor. Verifies that the AudioBuffer is not changed*/
    void writeBlock (AudioBuffer<float>& inputBuffer)
    {
        auto outputBuffer = tester->processBlock (processor, inputBuffer);
        // Assert the buffer hasn't changed after process()
        ASSERT_EQ (inputBuffer.getNumSamples(), outputBuffer.getNumSamples());
        ASSERT_EQ (inputBuffer.getNumChannels(), outputBuffer.getNumChannels());
        for (int chidx = 0; chidx < inputBuffer.getNumChannels(); chidx++)
        {
            for (int sampleIdx = 0; sampleIdx < inputBuffer.getNumSamples(); ++sampleIdx)
            {
                ASSERT_EQ (inputBuffer.getSample (chidx, sampleIdx), outputBuffer.getSample (chidx, sampleIdx));
            }
        }
        currentSampleIndex += inputBuffer.getNumSamples();
    }

    /*Gets information from canvas needed to build an ExpectedImage*/
    void setExpectedImageParameters (LfpViewer::LfpDisplayCanvas* canvas)
    {
        canvas->getChannelBitmapBounds (0, x, y, width, height);
        canvas->getChannelColours (0, channelColours, backgroundColour);
        ASSERT_EQ (channelColours.size(), numChannels);
    }

    /*Compares 2 Image objects and returns the number of different pixels*/
    int getImageDifferencePixelCount (Image expectedImage, Image actualImage) const
    {
        int missCount = 0;
        Image diffImage (Image::ARGB, width, height, true);
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                Colour expectedPixel = expectedImage.getPixelAt (x, y);
                Colour actualPixel = actualImage.getPixelAt (x, y);
                if (expectedPixel != actualPixel)
                {
                    //Allow some variance - if expectedPixel is a voltage Pixel then see if actualPixel +- 1 pixel is also a voltage Pixel.
                    //If true then don't count as a miss
                    if (expectedPixel != backgroundColour && expectedPixel != midlineColour && expectedPixel != playheadColour)
                    {
                        Colour postiveVariance = actualImage.getPixelAt (x, y + 1);
                        Colour negativeVariance = actualImage.getPixelAt (x, y - 1);
                        if (postiveVariance == expectedPixel || negativeVariance == expectedPixel)
                        {
                            continue;
                        }
                    }
                    //Same for if actualPixel is a voltagePixel and expectedPixel isn't
                    if (actualPixel != backgroundColour && actualPixel != midlineColour && actualPixel != playheadColour)
                    {
                        Colour postiveVariance = expectedImage.getPixelAt (x, y + 1);
                        Colour negativeVariance = expectedImage.getPixelAt (x, y - 1);
                        if (postiveVariance == actualPixel || negativeVariance == actualPixel)
                        {
                            continue;
                        }
                    }
                    missCount += 1;
                }
            }
        }
        return missCount;
    }

    void dumpPng (String path, Image image)
    {
        FileOutputStream stream ((File (path)));
        PNGImageFormat pngWriter;
        pngWriter.writeImageToStream (image, stream);
    }

    LfpViewer::LfpDisplayNode* processor;

    int numChannels;
    float bitVolts = 1.0;
    std::unique_ptr<ProcessorTester> tester;
    int64_t currentSampleIndex = 0;
    float sampleRate = 2000;

    Image expectedImage;
    Colour midlineColour;
    Colour backgroundColour;
    Colour playheadColour;

    Array<Colour> channelColours;

    int width;
    int height;
    int x;
    int y;
};

TEST_F (LfpDisplayNodeTests, VisualIntegrityTest)
{
    const int canvasX = 600;
    const int canvasY = 800;
    const float errorThreshold = 0.05f; //5% error threshold

    //Initialize LFP Canvas
    std::unique_ptr<LfpViewer::LfpDisplayCanvas> canvas = std::make_unique<LfpViewer::LfpDisplayCanvas> (processor, LfpViewer::SplitLayouts::SINGLE, false);
    canvas->updateSettings();
    canvas->setSize (canvasX, canvasY);
    canvas->resized();
    canvas->setVisible (true);

    //Get LFP size parameters from canvas
    setExpectedImageParameters (canvas.get());

    //Create snapshot of canvas channel bitmap and expected image
    Rectangle<int> canvasSnapshot (x, y, width, height);
    ExpectedImage expected (numChannels, sampleRate * 2); //2 seconds to match canvas timebase

    tester->startAcquisition (false);
    canvas->beginAnimation();

    //Add 5 10Hz waves with +-125uV amplitude
    auto inputBuffer = createBufferSinusoidal (5, numChannels, 1000, 125);
    writeBlock (inputBuffer);
    expected.addToBuffer (inputBuffer);
    canvas->refreshState();
    Image canvasImage = canvas->createComponentSnapshot (canvasSnapshot);
    Image expectedImage = expected.getImage (width, height, channelColours, backgroundColour, midlineColour, 125);
    int missCount = getImageDifferencePixelCount (expectedImage, canvasImage);
    EXPECT_LE (float(missCount) / float(width * height), errorThreshold);

    //Add 5 10Hz waves with +-250uV amplitude
    inputBuffer = createBufferSinusoidal (5, numChannels, 1000, 250);
    writeBlock (inputBuffer);
    expected.addToBuffer (inputBuffer);
    canvas->refreshState();

    canvasImage = canvas->createComponentSnapshot (canvasSnapshot);
    expectedImage = expected.getImage (width, height, channelColours, backgroundColour, midlineColour, 125);
    missCount = getImageDifferencePixelCount (expectedImage, canvasImage);
    EXPECT_LE (float (missCount) / float (width * height), errorThreshold);

    //Add 10 40Hz waves with +-250uV amplitude
    inputBuffer = createBufferSinusoidal (10, numChannels, 1000, 250);
    writeBlock (inputBuffer);
    expected.addToBuffer (inputBuffer);
    canvas->refreshState();

    canvasImage = canvas->createComponentSnapshot (canvasSnapshot);
    expectedImage = expected.getImage (width, height, channelColours, backgroundColour, midlineColour, 125);
    missCount = getImageDifferencePixelCount (expectedImage, canvasImage);
    EXPECT_LE (float (missCount) / float (width * height), errorThreshold);

    //Resize canvas to have half the vertical height and twice the uV range
    canvas->setChannelHeight (0, 20);
    canvas->setChannelRange (0, 500, ContinuousChannel::Type::ELECTRODE);
    canvas->refreshState();
    setExpectedImageParameters (canvas.get());

    canvasSnapshot.setBounds (x, y, width, height);

    canvasImage = canvas->createComponentSnapshot (canvasSnapshot);
    expectedImage = expected.getImage (width, height, channelColours, backgroundColour, midlineColour, 125);
    missCount = getImageDifferencePixelCount (expectedImage, canvasImage);
    EXPECT_LE (float (missCount) / float (width * height), errorThreshold);

    tester->stopAcquisition();
}

TEST_F (LfpDisplayNodeTests, DataIntegrityTest)
{
    int numSamples = 100;
    tester->startAcquisition (false);

    auto inputBuffer = createBuffer (1000.0, 20.0, numChannels, numSamples);
    writeBlock (inputBuffer);

    tester->stopAcquisition();
}
