#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class MockMetadataEventObject : public MetadataEventObject
{
public:
    MockMetadataEventObject() = default;
};

// Test fixture for MetadataEventObject
class MetadataEventObjectTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a MetadataEventObject instance
        metadataEventObject = new MockMetadataEventObject;
    }

    void TearDown() override
    {
        // Clean up the MetadataEventObject instance
        delete metadataEventObject;
    }

protected:
    MetadataEventObject* metadataEventObject;
};

// Test adding event metadata
TEST_F (MetadataEventObjectTests, AddEventMetadata)
{
    // Create a metadata descriptor
    MetadataDescriptor::MetadataType type = MetadataDescriptor::INT32;
    unsigned int length = 1;
    String name = "event_id";
    String description = "The ID of the event";
    String identifier = "metadata.event_id";
    MetadataDescriptor descriptor (type, length, name, description, identifier);

    // Add the metadata descriptor to the MetadataEventObject
    metadataEventObject->addEventMetadata (descriptor);

    // Check if the metadata descriptor was added successfully
    EXPECT_EQ (metadataEventObject->getEventMetadataCount(), 1);
    EXPECT_EQ (metadataEventObject->getEventMetadataDescriptor (0)->getName(), name);
    EXPECT_EQ (metadataEventObject->getEventMetadataDescriptor (0)->getDescription(), description);
    EXPECT_EQ (metadataEventObject->getEventMetadataDescriptor (0)->getIdentifier(), identifier);
}

// Test finding event metadata
TEST_F (MetadataEventObjectTests, FindEventMetadata)
{
    // Create metadata descriptors
    MetadataDescriptor::MetadataType type1 = MetadataDescriptor::INT32;
    unsigned int length1 = 1;
    String name1 = "event_id";
    String description1 = "The ID of the event";
    String identifier1 = "metadata.event_id";
    MetadataDescriptor descriptor1 (type1, length1, name1, description1, identifier1);

    MetadataDescriptor::MetadataType type2 = MetadataDescriptor::FLOAT;
    unsigned int length2 = 3;
    String name2 = "position";
    String description2 = "The position of the event";
    String identifier2 = "metadata.position";
    MetadataDescriptor descriptor2 (type2, length2, name2, description2, identifier2);

    // Add the metadata descriptors to the MetadataEventObject
    metadataEventObject->addEventMetadata (descriptor1);
    metadataEventObject->addEventMetadata (descriptor2);

    // Find the index of the metadata descriptors
    int index1 = metadataEventObject->findEventMetadata (type1, length1, identifier1);
    int index2 = metadataEventObject->findEventMetadata (type2, length2, identifier2);

    // Check if the index of the metadata descriptors was found correctly
    EXPECT_EQ (index1, 0);
    EXPECT_EQ (index2, 1);
}

// Test getting total event metadata size
TEST_F (MetadataEventObjectTests, GetTotalEventMetadataSize)
{
    // Create metadata descriptors
    MetadataDescriptor::MetadataType type1 = MetadataDescriptor::INT32;
    unsigned int length1 = 1;
    String name1 = "event_id";
    String description1 = "The ID of the event";
    String identifier1 = "metadata.event_id";
    MetadataDescriptor descriptor1 (type1, length1, name1, description1, identifier1);

    MetadataDescriptor::MetadataType type2 = MetadataDescriptor::FLOAT;
    unsigned int length2 = 3;
    String name2 = "position";
    String description2 = "The position of the event";
    String identifier2 = "metadata.position";
    MetadataDescriptor descriptor2 (type2, length2, name2, description2, identifier2);

    // Add the metadata descriptors to the MetadataEventObject
    metadataEventObject->addEventMetadata (descriptor1);
    metadataEventObject->addEventMetadata (descriptor2);

    // Calculate the total event metadata size
    size_t totalSize = metadataEventObject->getTotalEventMetadataSize();

    // Check if the total event metadata size was calculated correctly
    size_t expectedSize = descriptor1.getDataSize() + descriptor2.getDataSize();
    EXPECT_EQ (totalSize, expectedSize);
}

// Test getting the maximum event metadata size
TEST_F (MetadataEventObjectTests, GetMaxEventMetadataSize)
{
    // Create metadata descriptors
    MetadataDescriptor::MetadataType type1 = MetadataDescriptor::INT32;
    unsigned int length1 = 1;
    String name1 = "event_id";
    String description1 = "The ID of the event";
    String identifier1 = "metadata.event_id";
    MetadataDescriptor descriptor1 (type1, length1, name1, description1, identifier1);

    MetadataDescriptor::MetadataType type2 = MetadataDescriptor::FLOAT;
    unsigned int length2 = 3;
    String name2 = "position";
    String description2 = "The position of the event";
    String identifier2 = "metadata.position";
    MetadataDescriptor descriptor2 (type2, length2, name2, description2, identifier2);

    // Add the metadata descriptors to the MetadataEventObject
    metadataEventObject->addEventMetadata (descriptor1);
    metadataEventObject->addEventMetadata (descriptor2);

    // Get the maximum event metadata size
    size_t maxSize = metadataEventObject->getMaxEventMetadataSize();

    // Check if the maximum event metadata size was retrieved correctly
    size_t expectedSize = std::max (descriptor1.getDataSize(), descriptor2.getDataSize());
    EXPECT_EQ (maxSize, expectedSize);
}

// Test checking if two MetadataEventObjects have the same event metadata
TEST_F (MetadataEventObjectTests, HasSameEventMetadata)
{
    // Create metadata descriptors
    MetadataDescriptor::MetadataType type1 = MetadataDescriptor::INT32;
    unsigned int length1 = 1;
    String name1 = "event_id";
    String description1 = "The ID of the event";
    String identifier1 = "metadata.event_id";
    MetadataDescriptor descriptor1 (type1, length1, name1, description1, identifier1);

    MetadataDescriptor::MetadataType type2 = MetadataDescriptor::FLOAT;
    unsigned int length2 = 3;
    String name2 = "position";
    String description2 = "The position of the event";
    String identifier2 = "metadata.position";
    MetadataDescriptor descriptor2 (type2, length2, name2, description2, identifier2);

    // Add the metadata descriptors to the MetadataEventObjects
    metadataEventObject->addEventMetadata (descriptor1);
    metadataEventObject->addEventMetadata (descriptor2);

    MockMetadataEventObject otherMetadataEventObject;

    // Add the same metadata descriptors to the other MetadataEventObject
    otherMetadataEventObject.addEventMetadata (descriptor1);
    otherMetadataEventObject.addEventMetadata (descriptor2);

    // Check if the two MetadataEventObjects have the same event metadata
    EXPECT_TRUE (metadataEventObject->hasSameEventMetadata (otherMetadataEventObject));
}

// Test checking if two MetadataEventObjects have similar event metadata
TEST_F (MetadataEventObjectTests, HasSimilarEventMetadata)
{
    // Create metadata descriptors
    MetadataDescriptor::MetadataType type1 = MetadataDescriptor::INT32;
    unsigned int length1 = 1;
    String name1 = "event_id";
    String description1 = "The ID of the event";
    String identifier1 = "metadata.event_id";
    MetadataDescriptor descriptor1 (type1, length1, name1, description1, identifier1);

    MetadataDescriptor::MetadataType type2 = MetadataDescriptor::FLOAT;
    unsigned int length2 = 3;
    String name2 = "position";
    String description2 = "The position of the event";
    String identifier2 = "metadata.position";
    MetadataDescriptor descriptor2 (type2, length2, name2, description2, identifier2);

    // Add the metadata descriptors to the MetadataEventObjects
    metadataEventObject->addEventMetadata (descriptor1);
    metadataEventObject->addEventMetadata (descriptor2);

    MockMetadataEventObject otherMetadataEventObject;

    // Add similar metadata descriptors to the other MetadataEventObject
    MetadataDescriptor::MetadataType similarType1 = MetadataDescriptor::INT32;
    unsigned int similarLength1 = 1;
    String similarName1 = "event_id";
    String similarDescription1 = "The ID of the event";
    String similarIdentifier1 = "metadata.event_id";
    MetadataDescriptor similarDescriptor1 (similarType1, similarLength1, similarName1, similarDescription1, similarIdentifier1);

    MetadataDescriptor::MetadataType similarType2 = MetadataDescriptor::FLOAT;
    unsigned int similarLength2 = 3;
    String similarName2 = "position";
    String similarDescription2 = "The position of the event";
    String similarIdentifier2 = "metadata.position";
    MetadataDescriptor similarDescriptor2 (similarType2, similarLength2, similarName2, similarDescription2, similarIdentifier2);

    otherMetadataEventObject.addEventMetadata (similarDescriptor1);
    otherMetadataEventObject.addEventMetadata (similarDescriptor2);

    // Check if the two MetadataEventObjects have similar event metadata
    EXPECT_TRUE (metadataEventObject->hasSimilarEventMetadata (otherMetadataEventObject));
}