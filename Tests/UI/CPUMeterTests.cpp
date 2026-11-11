#include "UI/ControlPanel.h"
#include "gtest/gtest.h"

class CPUMeterTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        meter.setSize (200, 16);
        meter.setColour (ThemeColours::defaultFill, Colours::black);
    }

    Image render()
    {
        Image image (Image::RGB, meter.getWidth(), meter.getHeight(), true);
        Graphics graphics (image);
        meter.paint (graphics);
        return image;
    }

    ScopedJuceInitialiser_GUI juce;
    CPUMeter meter;
};

TEST_F (CPUMeterTests, SmoothsColourWithoutSmoothingMeterWidth)
{
    for (int i = 0; i < 4; ++i)
        meter.updateCPU (0.6f);

    meter.updateCPU (1.0f);
    EXPECT_EQ (render().getPixelAt (180, 5), Colours::yellow);

    meter.updateCPU (0.2f);
    const auto image = render();
    EXPECT_EQ (image.getPixelAt (10, 5), Colours::yellow);
    EXPECT_EQ (image.getPixelAt (60, 5), Colours::black);
}

TEST_F (CPUMeterTests, ReplacesOldReadingsAfterFourUpdates)
{
    for (int i = 0; i < 4; ++i)
        meter.updateCPU (0.6f);

    for (int i = 0; i < 3; ++i)
        meter.updateCPU (1.0f);

    EXPECT_EQ (render().getPixelAt (10, 5),
               Colours::yellow.interpolatedWith (Colours::red, (0.9f - 0.8f) * 5.0f));

    meter.updateCPU (1.0f);
    EXPECT_EQ (render().getPixelAt (10, 5), Colours::red);

    for (int i = 0; i < 4; ++i)
        meter.updateCPU (0.6f);

    EXPECT_EQ (render().getPixelAt (10, 5), Colours::yellow);
}

TEST_F (CPUMeterTests, ResetClearsPreviousAcquisitionReadings)
{
    meter.updateCPU (1.0f);
    EXPECT_EQ (render().getPixelAt (10, 5), Colours::red);

    meter.reset();
    EXPECT_EQ (render().getPixelAt (10, 5), Colours::black);

    meter.updateCPU (0.4f);
    EXPECT_EQ (render().getPixelAt (10, 5), Colours::yellow);
}
