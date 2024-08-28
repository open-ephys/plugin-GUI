#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class MockMetadataEvent : public MetadataEvent
{
public:
    MockMetadataEvent()
    {
        m_metaDataValues.add (new MetadataValue (MetadataDescriptor::MetadataType::CHAR, 5));
    }
};

class MetadataEventTests : public testing::Test
{
protected:
    void SetUp() override
    {
        metadataEvent = std::make_unique<MockMetadataEvent>();
    }

protected:
    std::unique_ptr<MockMetadataEvent> metadataEvent;
};

TEST_F(MetadataEventTests, getMetadataValueCount)
{
    EXPECT_EQ(metadataEvent->getMetadataValueCount(), 1);
}

TEST_F(MetadataEventTests, getMetadataValue)
{
    const MetadataValue* value = metadataEvent->getMetadataValue(0);
    EXPECT_EQ (value->getDataType(), MetadataDescriptor::MetadataType::CHAR);
    EXPECT_EQ (value->getDataSize(), 6);
    EXPECT_EQ (value->getDataLength(), 5);
}