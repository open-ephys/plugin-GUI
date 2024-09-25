#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class MockParameterOwner : public ParameterOwner
{
public:
    MockParameterOwner() :
        ParameterOwner(Type::CONTINUOUS_CHANNEL)
    {
        addParameter(new IntParameter(this,
                                      Parameter::PROCESSOR_SCOPE,
                                      "Int",
                                      "Int",
                                      "Int",
                                      0));

        addParameter(new FloatParameter(this,
                                        Parameter::PROCESSOR_SCOPE,
                                        "Float",
                                        "Float",
                                        "Float",
                                        "Float",
                                        0.0f));

        addParameter(new BooleanParameter(this,
                                          Parameter::PROCESSOR_SCOPE,
                                          "Bool",
                                          "Bool",
                                          "Bool",
                                          false));

        addParameter (new StringParameter (this,
                                           Parameter::PROCESSOR_SCOPE,
                                           "String",
                                           "String",
                                           "String",
                                           "String"));
    }
};

class ParameterOwnerTests : public testing::Test
{
protected:
    void SetUp() override
    {
        owner = std::make_shared<MockParameterOwner>();
    }

protected:
    std::shared_ptr<MockParameterOwner> owner;
};

TEST_F(ParameterOwnerTests, addGetParameter)
{
    EXPECT_EQ (owner->getParameter ("Int")->getName(), "Int");
    EXPECT_TRUE (owner->getParameter ("Int")->getDefaultValue().equals(var(0)));
    EXPECT_TRUE (owner->getParameter ("Int")->getValue().equals(var(0)));
    EXPECT_EQ (owner->getParameter ("Int")->getScope(), Parameter::PROCESSOR_SCOPE);
    EXPECT_EQ (owner->getParameter ("Int")->getOwner(), owner.get());
    EXPECT_EQ (owner->getParameter ("Int")->getDisplayName(), "Int");
    EXPECT_EQ (owner->getParameter ("Int")->getDescription(), "Int");

    EXPECT_EQ (owner->getParameter ("Float")->getName(), "Float");
    EXPECT_TRUE (owner->getParameter ("Float")->getDefaultValue().equals(var(0.0f)));
    EXPECT_TRUE (owner->getParameter ("Float")->getValue().equals(var(0.0f)));
    EXPECT_EQ (owner->getParameter ("Float")->getScope(), Parameter::PROCESSOR_SCOPE);
    EXPECT_EQ (owner->getParameter ("Float")->getOwner(), owner.get());
    EXPECT_EQ (owner->getParameter ("Float")->getDisplayName(), "Float");
    EXPECT_EQ (owner->getParameter ("Float")->getDescription(), "Float");

    EXPECT_EQ (owner->getParameter ("Bool")->getName(), "Bool");
    EXPECT_TRUE (owner->getParameter ("Bool")->getDefaultValue().equals(var(false)));
    EXPECT_TRUE (owner->getParameter ("Bool")->getValue().equals(var(false)));
    EXPECT_EQ (owner->getParameter ("Bool")->getScope(), Parameter::PROCESSOR_SCOPE);
    EXPECT_EQ (owner->getParameter ("Bool")->getOwner(), owner.get());
    EXPECT_EQ (owner->getParameter ("Bool")->getDisplayName(), "Bool");
    EXPECT_EQ (owner->getParameter ("Bool")->getDescription(), "Bool");

    EXPECT_EQ (owner->getParameter ("String")->getName(), "String");
    EXPECT_TRUE (owner->getParameter ("String")->getDefaultValue().equals(var("String")));
    EXPECT_TRUE (owner->getParameter ("String")->getValue().equals(var("String")));
    EXPECT_EQ (owner->getParameter ("String")->getScope(), Parameter::PROCESSOR_SCOPE);
    EXPECT_EQ (owner->getParameter ("String")->getOwner(), owner.get());
    EXPECT_EQ (owner->getParameter ("String")->getDisplayName(), "String");
    EXPECT_EQ (owner->getParameter ("String")->getDescription(), "String");
}

TEST_F(ParameterOwnerTests, hasParameter)
{
    EXPECT_TRUE (owner->hasParameter ("Int"));
    EXPECT_TRUE (owner->hasParameter ("Float"));
    EXPECT_TRUE (owner->hasParameter ("Bool"));
    EXPECT_TRUE (owner->hasParameter ("String"));
}

TEST_F(ParameterOwnerTests, getParameters)
{
    auto parameters = owner->getParameters();
    EXPECT_EQ (parameters.size(), 4);
}

TEST_F(ParameterOwnerTests, getParameterNames)
{
    auto names = owner->getParameterNames();
    EXPECT_EQ (names.size(), 4);
    EXPECT_EQ (names[0], String ("Int"));
    EXPECT_EQ (names[1], String ("Float"));
    EXPECT_EQ (names[2], String ("Bool"));
    EXPECT_EQ (names[3], String ("String"));
}

TEST_F(ParameterOwnerTests, copyParameters)
{
    MockParameterOwner other;
    owner->copyParameters (&other);

    auto parameters = other.getParameters();
    EXPECT_EQ (parameters.size(), 4);
    EXPECT_EQ (parameters[0]->getName(), String("Int"));
    EXPECT_EQ (parameters[1]->getName(), String("Float"));
    EXPECT_EQ (parameters[2]->getName(), String("Bool"));
    EXPECT_EQ (parameters[3]->getName(), String("String"));
}

TEST_F(ParameterOwnerTests, bracketOperator)
{
    EXPECT_TRUE (owner->operator[] ("Int").equals(var(0)));
    EXPECT_TRUE (owner->operator[] ("Float").equals(var(0.0f)));
    EXPECT_TRUE (owner->operator[] ("Bool").equals(var(false)));
    EXPECT_TRUE (owner->operator[] ("String").equals(var("String")));
}

TEST_F(ParameterOwnerTests, setGetColour)
{
    owner->setColour ("Int", Colours::red);
    owner->setColour ("Float", Colours::green);
    owner->setColour ("Bool", Colours::blue);
    owner->setColour ("String", Colours::yellow);

    EXPECT_EQ (owner->getColour ("Int"), Colours::red);
    EXPECT_EQ (owner->getColour ("Float"), Colours::green);
    EXPECT_EQ (owner->getColour ("Bool"), Colours::blue);
    EXPECT_EQ (owner->getColour ("String"), Colours::yellow);
}

TEST_F(ParameterOwnerTests, numParameters)
{
    EXPECT_EQ (owner->numParameters(), 4);
}

TEST_F(ParameterOwnerTests, customCopyConstructor)
{
    MockParameterOwner other (*owner);
    auto parameters = other.getParameters();
    EXPECT_EQ (parameters.size(), 0);
    EXPECT_EQ (other.getType(), ParameterOwner::Type::CONTINUOUS_CHANNEL);
}

TEST_F(ParameterOwnerTests, getType)
{
    EXPECT_EQ (owner->getType(), ParameterOwner::Type::CONTINUOUS_CHANNEL);
}