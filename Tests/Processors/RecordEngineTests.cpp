#include "gtest/gtest.h"

#include <Processors/RecordNode/RecordEngine.h>
#include <TestFixtures.h>
#include <map>

namespace
{
class ParameterCapturingEngine : public RecordEngine
{
public:
    String getEngineId() const override { return "BINARY"; }
    void openFiles (File, int, int) override { valuesAtOpen = values; }
    void closeFiles() override {}
    void writeContinuousData (int, int, const float*, const double*, int) override {}
    void writeEvent (int, const EventPacket&) override {}
    void writeSpike (int, const Spike*) override {}
    void writeTimestampSyncText (uint64, int64, float, String) override {}

    void setParameter (EngineParameter& parameter) override
    {
        switch (parameter.type)
        {
            case EngineParameter::BOOL: values[parameter.id] = parameter.boolParam.value; break;
            case EngineParameter::INT: values[parameter.id] = parameter.intParam.value; break;
            case EngineParameter::FLOAT: values[parameter.id] = parameter.floatParam.value; break;
            case EngineParameter::STR: values[parameter.id] = parameter.strParam.value; break;
            case EngineParameter::MULTI: values[parameter.id] = parameter.multiParam.value; break;
        }
    }

    std::map<int, var> values;
    std::map<int, var> valuesAtOpen;
};

template <typename T>
T* findChild (Component& component, const String& name = {})
{
    if (auto* result = dynamic_cast<T*> (&component))
        if (name.isEmpty() || result->getName() == name)
            return result;
    for (auto* child : component.getChildren())
        if (auto* result = findChild<T> (*child, name))
            return result;
    return nullptr;
}

CallOutBox* findOptionsPopup (RecordNode& node)
{
    auto& desktop = Desktop::getInstance();
    for (int i = 0; i < desktop.getNumComponents(); ++i)
        if (auto* popup = dynamic_cast<CallOutBox*> (desktop.getComponent (i)))
            if (popup->isVisible() && popup->getName().startsWith ("Record Node " + String (node.getNodeId()) + " - "))
                return popup;
    return nullptr;
}

class RecordEngineOptionsTests : public testing::Test
{
protected:
    void SetUp() override
    {
        tester = std::make_unique<ProcessorTester> (
            TestSourceNodeBuilder (FakeSourceNodeParams { 2, 30000.0f, 1.0f }), ProcessorTesterMode::FullApp);
        auto* manager = binaryManager();
        ASSERT_NE (manager, nullptr);
        manager->addParameter (new EngineParameter (EngineParameter::INT, 7, "Integer", 2, -10, 10));
        manager->addParameter (new EngineParameter (EngineParameter::FLOAT, 11, "Float", 0.25, -1.0, 1.0));
        manager->addParameter (new EngineParameter (EngineParameter::STR, 19, "Identifier", "default"));
        manager->addParameter (new EngineParameter (EngineParameter::MULTI, 23, "Choice index", 0));
        node = tester->createProcessor<RecordNode> (Plugin::Processor::RECORD_NODE);
    }

    RecordEngineManager* binaryManager()
    {
        for (auto* manager : tester->controlPanel->getAvailableRecordEngines())
            if (manager->getID() == "BINARY")
                return manager;
        return nullptr;
    }

    Parameter* parameter (RecordNode& target, int id)
    {
        return target.getParameter (RecordNode::getEngineParameterName ("BINARY", id));
    }

    ParameterCapturingEngine* captureEngine (RecordNode& target)
    {
        auto capture = std::make_unique<ParameterCapturingEngine>();
        capture->registerManager (binaryManager());
        auto* result = capture.get();
        target.recordEngine = std::move (capture);
        target.recordThread->setEngine (target.recordEngine.get());
        return result;
    }

    std::unique_ptr<ProcessorTester> tester;
    RecordNode* node = nullptr;
};
}

TEST (RecordEngineParameters, FactoryRegistersManagerAndAppliesStandaloneDefaults)
{
    RecordEngineManager manager ("BINARY", "Parameter test", &engineFactory<ParameterCapturingEngine>);
    manager.addParameter (new EngineParameter (EngineParameter::BOOL, 0, "Boolean", true));
    std::unique_ptr<RecordEngine> engine (manager.instantiateEngine());
    ASSERT_NE (engine, nullptr);
    engine->configureEngine();
    auto* capture = dynamic_cast<ParameterCapturingEngine*> (engine.get());
    ASSERT_NE (capture, nullptr);
    ASSERT_EQ (capture->values.size(), 1);
    EXPECT_TRUE (bool (capture->values.at (0)));
}

TEST_F (RecordEngineOptionsTests, NodesApplyIndependentValuesBeforeOpeningFiles)
{
    XmlElement defaults ("ENGINE");
    binaryManager()->saveParametersToXml (&defaults);
    auto* second = tester->createProcessor<RecordNode> (Plugin::Processor::RECORD_NODE);
    parameter (*node, 0)->setNextValue (false, false);
    parameter (*node, 7)->setNextValue (-4, false);
    parameter (*node, 11)->setNextValue (0.75, false);
    parameter (*node, 19)->setNextValue ("node-one", false);
    parameter (*node, 23)->setNextValue (2, false);
    parameter (*second, 19)->setNextValue ("node-two", false);
    auto* firstEngine = captureEngine (*node);
    auto* secondEngine = captureEngine (*second);

    tester->startAcquisition (true, true);
    tester->stopAcquisition();
    ASSERT_EQ (firstEngine->valuesAtOpen.size(), binaryManager()->getNumParameters());
    ASSERT_EQ (secondEngine->valuesAtOpen.size(), binaryManager()->getNumParameters());
    EXPECT_FALSE (bool (firstEngine->valuesAtOpen.at (0)));
    EXPECT_EQ (int (firstEngine->valuesAtOpen.at (7)), -4);
    EXPECT_DOUBLE_EQ (double (firstEngine->valuesAtOpen.at (11)), 0.75);
    EXPECT_EQ (firstEngine->valuesAtOpen.at (19).toString(), "node-one");
    EXPECT_EQ (int (firstEngine->valuesAtOpen.at (23)), 2);
    EXPECT_TRUE (bool (secondEngine->valuesAtOpen.at (0)));
    EXPECT_EQ (secondEngine->valuesAtOpen.at (19).toString(), "node-two");
    XmlElement afterRecording ("ENGINE");
    binaryManager()->saveParametersToXml (&afterRecording);
    EXPECT_TRUE (defaults.isEquivalentTo (&afterRecording, false));
}

TEST_F (RecordEngineOptionsTests, ParameterValuesRoundTripWithNodeSettings)
{
    parameter (*node, 0)->setNextValue (false, false);
    parameter (*node, 7)->setNextValue (-4, false);
    parameter (*node, 11)->setNextValue (-0.75, false);
    parameter (*node, 19)->setNextValue ("identifier & metadata", false);
    parameter (*node, 23)->setNextValue (2, false);
    XmlElement saved ("PROCESSOR");
    node->saveToXml (&saved);

    auto* second = tester->createProcessor<RecordNode> (Plugin::Processor::RECORD_NODE);
    second->parametersAsXml = &saved;
    second->loadFromXml();
    second->parametersAsXml = nullptr;
    for (const int id : { 0, 7, 11, 19, 23 })
        EXPECT_EQ (parameter (*second, id)->getValue(), parameter (*node, id)->getValue()) << "Parameter " << id;
}

TEST_F (RecordEngineOptionsTests, PopupCommitsPendingTextBeforeSavingAndClosesForAcquisition)
{
    node->showEngineOptions();
    auto* popup = findOptionsPopup (*node);
    ASSERT_NE (popup, nullptr);
    auto* text = findChild<Label> (*popup, parameter (*node, 19)->getKey());
    ASSERT_NE (text, nullptr);
    text->showEditor();
    ASSERT_NE (text->getCurrentTextEditor(), nullptr);
    text->getCurrentTextEditor()->setText ("pending identifier");
    std::unique_ptr<XmlElement> saved (tester->processorGraph->createNodeXml (node, false));
    EXPECT_EQ (parameter (*node, 19)->getValue().toString(), "pending identifier");
    ASSERT_NE (saved->getChildByName ("PROCESSOR_PARAMETERS"), nullptr);
    EXPECT_EQ (saved->getChildByName ("PROCESSOR_PARAMETERS")->getStringAttribute ("engine_BINARY_19"), "pending identifier");

    tester->startAcquisition (false);
    EXPECT_EQ (findOptionsPopup (*node), nullptr);
    node->showEngineOptions();
    EXPECT_EQ (findOptionsPopup (*node), nullptr);
    tester->stopAcquisition();
}

TEST_F (RecordEngineOptionsTests, DeletingNodeReleasesPopupBeforeAsyncDismissal)
{
    auto temporary = std::make_unique<RecordNode>();
    temporary->setNodeId (999);
    temporary->setHeadlessMode (true);
    temporary->registerParameters();
    temporary->showEngineOptions();
    auto* popup = findOptionsPopup (*temporary);
    ASSERT_NE (popup, nullptr);
    auto* text = findChild<Label> (*popup, parameter (*temporary, 19)->getKey());
    ASSERT_NE (text, nullptr);
    text->showEditor();
    ASSERT_NE (text->getCurrentTextEditor(), nullptr);
    text->getCurrentTextEditor()->setText ("pending on node deletion");
    Component::SafePointer<CallOutBox> previous = popup;
    temporary.reset();
    const auto deadline = Time::getMillisecondCounterHiRes() + 2000.0;
    while (previous != nullptr && Time::getMillisecondCounterHiRes() < deadline)
        MessageManager::getInstance()->runDispatchLoopUntil (10);
    EXPECT_EQ (previous.getComponent(), nullptr);
}

TEST_F (RecordEngineOptionsTests, PopupScrollsOnlyWhenEditorsExceedScreenHeight)
{
    node->showEngineOptions();
    auto* popup = findOptionsPopup (*node);
    ASSERT_NE (popup, nullptr);
    auto* viewport = findChild<Viewport> (*popup);
    ASSERT_NE (viewport, nullptr);
    ASSERT_NE (viewport->getViewedComponent(), nullptr);
    EXPECT_EQ (viewport->getHeight(), viewport->getViewedComponent()->getHeight());
    EXPECT_FALSE (viewport->getVerticalScrollBar().isVisible());
    auto* editor = findChild<ParameterEditor> (*viewport);
    ASSERT_NE (editor, nullptr);
    ASSERT_GT (editor->getHeight(), 0);
    const auto screen = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
    const int extraParameters = screen.getHeight() / editor->getHeight() + 1;
    node->closeEngineOptions();

    for (int i = 0; i < extraParameters; ++i)
        binaryManager()->addParameter (new EngineParameter (EngineParameter::STR, 100 + i, "Metadata " + String (i), ""));
    auto* second = tester->createProcessor<RecordNode> (Plugin::Processor::RECORD_NODE);
    second->showEngineOptions();
    popup = findOptionsPopup (*second);
    ASSERT_NE (popup, nullptr);
    viewport = findChild<Viewport> (*popup);
    ASSERT_NE (viewport, nullptr);
    ASSERT_NE (viewport->getViewedComponent(), nullptr);
    EXPECT_TRUE (screen.contains (popup->getScreenBounds()));
    EXPECT_GT (viewport->getViewedComponent()->getHeight(), viewport->getHeight());
    EXPECT_TRUE (viewport->getVerticalScrollBar().isVisible());
    EXPECT_FALSE (viewport->getHorizontalScrollBar().isVisible());
    second->closeEngineOptions();
}
