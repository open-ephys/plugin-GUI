#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class MockInfoObject : public InfoObject
{
public:
    MockInfoObject (Type type) : InfoObject (type) {};
    ~MockInfoObject() override {};

    void setSourceNodeId (int nodeId)
    {
        InfoObject::setSourceNodeId (nodeId);
    }

    void setSourceNodeName (String nodeName)
    {
        InfoObject::setSourceNodeName (nodeName);
    }
};

class MockGenericProcessor : public GenericProcessor
{
public:
    MockGenericProcessor (const String& name, bool headlessMode = false) : 
        GenericProcessor(name, headlessMode) {};
    ~MockGenericProcessor() override {};


    // Inherited via GenericProcessor
    void process (AudioBuffer<float>& continuousBuffer) override
    {
    }
};

class InfoObjectTests : public testing::Test
{
protected:
    void SetUp() override
    {
        infoObject = new MockInfoObject (InfoObject::Type::CONTINUOUS_CHANNEL);
        processor = new MockGenericProcessor ("Processor1");
    }

    void TearDown() override
    {
        delete infoObject;
        delete processor;
    }

protected:
    MockInfoObject* infoObject;
    GenericProcessor* processor;
};

// Test the getType() method
TEST_F (InfoObjectTests, GetTypeTest)
{
    EXPECT_EQ (infoObject->getType(), InfoObject::Type::CONTINUOUS_CHANNEL);
}

// Test the getLocalIndex() and setLocalIndex() methods
TEST_F (InfoObjectTests, LocalIndexTest)
{
    infoObject->setLocalIndex (5);
    EXPECT_EQ (infoObject->getLocalIndex(), 5);
}

// Test the getGlobalIndex() and setGlobalIndex() methods
TEST_F (InfoObjectTests, GlobalIndexTest)
{
    infoObject->setGlobalIndex (10);
    EXPECT_EQ (infoObject->getGlobalIndex(), 10);
}

// Test the getNodeId() and setNodeId() methods
TEST_F (InfoObjectTests, NodeIdTest)
{
    infoObject->setNodeId (15);
    EXPECT_EQ (infoObject->getNodeId(), 15);
}

// Test the getNodeName() method
TEST_F (InfoObjectTests, NodeNameTest)
{
    infoObject->addProcessor (processor);
    EXPECT_EQ (infoObject->getNodeName(), String("Processor1"));
}

// Test the getSourceNodeId() method
TEST_F (InfoObjectTests, SourceNodeIdTest)
{
    infoObject->setSourceNodeId (20);
    EXPECT_EQ (infoObject->getSourceNodeId(), 20);
}

// Test the getSourceNodeName() method
TEST_F (InfoObjectTests, SourceNodeNameTest)
{
    infoObject->setSourceNodeName ("Processor2");
    EXPECT_EQ (infoObject->getSourceNodeName(), "Processor2");
}

// Test the addProcessor() method
TEST_F (InfoObjectTests, AddProcessorTest)
{
    infoObject->addProcessor (processor);
    EXPECT_EQ (infoObject->processorChain.size(), 1);
    EXPECT_EQ (infoObject->processorChain[0], processor);
}

// Test the isLocal() method
TEST_F (InfoObjectTests, IsLocalTest)
{
    EXPECT_TRUE (infoObject->isLocal());
}

TEST_F (InfoObjectTests, CopyConstructorTest)
{
    infoObject->addProcessor (processor);
    String sourceNodeName = "Processor2";
    infoObject->setSourceNodeName (sourceNodeName);
    infoObject->setNodeId (25);
    infoObject->setSourceNodeId (30);

    MockInfoObject copy (*infoObject);
    EXPECT_EQ (copy.getType(), InfoObject::Type::CONTINUOUS_CHANNEL);
    EXPECT_EQ (copy.getLocalIndex(), -1);
    EXPECT_EQ (copy.getGlobalIndex(), -1);
    EXPECT_EQ (copy.getNodeId(), 25);
    EXPECT_EQ (copy.getNodeName(), processor->getName());
    EXPECT_EQ (copy.getSourceNodeId(), 30);
    EXPECT_EQ (copy.getSourceNodeName(), sourceNodeName);
    EXPECT_EQ (copy.processorChain.size(), 1);
    EXPECT_FALSE (copy.isLocal());
}