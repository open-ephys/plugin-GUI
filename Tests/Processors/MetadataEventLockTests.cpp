#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class MockMetadataEventLock : public MetadataEventLock
{
public:
    MockMetadataEventLock() = default;

    bool getEventMetadataLock() const
    {
        return eventMetadataLock;
    }

    void setEventMetadataLock (bool lock)
    {
        eventMetadataLock = lock;
    }
};

class MetadataEventLockTests : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create a MetadataEventLock instance
        metadataEventLock = new MockMetadataEventLock;
    }

    void TearDown() override
    {
        // Clean up the MetadataEventLock instance
        delete metadataEventLock;
    }

protected:
    MockMetadataEventLock* metadataEventLock;
};

TEST_F (MetadataEventLockTests, EventMetadataLock_DefaultValue)
{
    EXPECT_FALSE (metadataEventLock->getEventMetadataLock());
}

TEST_F (MetadataEventLockTests, EventMetadataLock_SetValue)
{
    metadataEventLock->setEventMetadataLock(true);
    EXPECT_TRUE (metadataEventLock->getEventMetadataLock());
}