#include "gtest/gtest.h"
#include <Audio/AudioComponent.h>
#include <Processors/ProcessorGraph/ProcessorGraph.h>
#include <UI/ControlPanel.h>
#include <modules/juce_gui_basics/juce_gui_basics.h>

class ProcessorGraphTest : public testing::Test
{
protected:
    void SetUp() override
    {
        MessageManager::deleteInstance();
        MessageManager::getInstance();
        AccessClass::clearAccessClassStateForTesting();

        customLookAndFeel = std::make_unique<CustomLookAndFeel>();
        LookAndFeel::setDefaultLookAndFeel (customLookAndFeel.get());

        // All of these sets the global state in AccessClass in their constructors
        audioComponent = std::make_unique<AudioComponent>();
        processorGraph = std::make_unique<ProcessorGraph> (true);
        controlPanel = std::make_unique<ControlPanel> (processorGraph.get(), audioComponent.get(), true);
    }

    void TearDown() override
    {
        controlPanel = nullptr;
        processorGraph = nullptr;
        audioComponent = nullptr;

        AccessClass::clearAccessClassStateForTesting();

        DeletedAtShutdown::deleteAll();
        MessageManager::deleteInstance();
    }

    std::unique_ptr<ProcessorGraph> processorGraph;

private:
    std::unique_ptr<AudioComponent> audioComponent;
    std::unique_ptr<ControlPanel> controlPanel;
    std::unique_ptr<CustomLookAndFeel> customLookAndFeel;
};

TEST_F (ProcessorGraphTest, LoadFromXMLTest)
{
    std::string defaultFileReaderPath = "default";

    File defaultFileReaderPathResources = File (String (RESOURCES_DIRECTORY)).getChildFile ("FileReader/resources/structure.oebin");

    if (defaultFileReaderPathResources.existsAsFile())
    {
        defaultFileReaderPath = defaultFileReaderPathResources.getFullPathName().toStdString();
    }

    // An example XML from a real run of OpenEphys. To generate this, we opened OpenEphys, dragged a FileReader and
    // BandpassFilter, and then did File > Save As... and saved the XML. The XML contents were then copied here.
    std::string docText = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                          "\n"
                          "<SETTINGS>\n"
                          "  <INFO>\n"
                          "    <VERSION>0.6.4</VERSION>\n"
                          "    <PLUGIN_API_VERSION>8</PLUGIN_API_VERSION>\n"
                          "    <DATE>17 May 2023 12:50:55</DATE>\n"
                          "    <OS>Mac OSX 12.6.1</OS>\n"
                          "    <MACHINE name=\"XXXX-MBP-2\" cpu_model=\"Apple M1 Pro\" cpu_num_cores=\"10\"/>\n"
                          "  </INFO>\n"
                          "  <SIGNALCHAIN>\n"
                          "    <PROCESSOR name=\"File Reader\" insertionPoint=\"0\" pluginName=\"File Reader\"\n"
                          "               type=\"0\" index=\"2\" libraryName=\"\" libraryVersion=\"\" processorType=\"2\"\n"
                          "               nodeId=\"100\">\n"
                          "      <GLOBAL_PARAMETERS/>\n"
                          "      <STREAM name=\"example_data\" description=\"A description of the File Reader Stream\"\n"
                          "              sample_rate=\"40000.0\" channel_count=\"16\">\n"
                          "        <PARAMETERS/>\n"
                          "      </STREAM>\n"
                          "      <CUSTOM_PARAMETERS>\n"
                          "        <FILENAME path=\""
                          + defaultFileReaderPath + "\" recording=\"0\"/>\n"
                                                    "      </CUSTOM_PARAMETERS>\n"
                                                    "      <EDITOR isCollapsed=\"0\" isDrawerOpen=\"0\" displayName=\"File Reader\" activeStream=\"0\">\n"
                                                    "        <TIME_LIMITS start_time=\"0.0\" stop_time=\"4999.0\"/>\n"
                                                    "      </EDITOR>\n"
                                                    "    </PROCESSOR>\n"
                                                    "    <PROCESSOR name=\"Bandpass Filter\" insertionPoint=\"1\" pluginName=\"Bandpass Filter\"\n"
                                                    "               type=\"1\" index=\"0\" libraryName=\"Bandpass Filter\" libraryVersion=\"0.1.0\"\n"
                                                    "               processorType=\"1\" nodeId=\"101\">\n"
                                                    "      <GLOBAL_PARAMETERS/>\n"
                                                    "      <STREAM name=\"example_data\" description=\"A description of the File Reader Stream\"\n"
                                                    "              sample_rate=\"40000.0\" channel_count=\"16\">\n"
                                                    "        <PARAMETERS enable_stream=\"1\" low_cut=\"300.0\" high_cut=\"6000.0\" Channels=\"\"/>\n"
                                                    "      </STREAM>\n"
                                                    "      <CUSTOM_PARAMETERS/>\n"
                                                    "      <EDITOR isCollapsed=\"0\" isDrawerOpen=\"0\" displayName=\"Bandpass Filter\"\n"
                                                    "              activeStream=\"0\"/>\n"
                                                    "    </PROCESSOR>\n"
                                                    "  </SIGNALCHAIN>\n"
                                                    "  <CONTROLPANEL isOpen=\"0\" recordPath=\"\" recordEngine=\"BINARY\"\n"
                                                    "                clockMode=\"0\"/>\n"
                                                    "  <AUDIOEDITOR isMuted=\"0\" volume=\"50.0\" noiseGate=\"0.0\"/>\n"
                                                    "  <FILENAMECONFIG>\n"
                                                    "    <PREPEND state=\"0\" value=\"\"/>\n"
                                                    "    <MAIN state=\"1\" value=\"YYYY-MM-DD_HH-MM-SS\"/>\n"
                                                    "    <APPEND state=\"0\" value=\"\"/>\n"
                                                    "  </FILENAMECONFIG>\n"
                                                    "  <EDITORVIEWPORT scroll=\"0\">\n"
                                                    "    <FILE_READER ID=\"100\"/>\n"
                                                    "    <BANDPASS_FILTER ID=\"101\"/>\n"
                                                    "  </EDITORVIEWPORT>\n"
                                                    "  <DATAVIEWPORT selectedTab=\"2\"/>\n"
                                                    "  <PROCESSORLIST>\n"
                                                    "    <COLOR ID=\"801\" R=\"59\" G=\"59\" B=\"59\"/>\n"
                                                    "    <COLOR ID=\"804\" R=\"241\" G=\"90\" B=\"41\"/>\n"
                                                    "    <COLOR ID=\"802\" R=\"0\" G=\"174\" B=\"239\"/>\n"
                                                    "    <COLOR ID=\"803\" R=\"0\" G=\"166\" B=\"81\"/>\n"
                                                    "    <COLOR ID=\"805\" R=\"147\" G=\"149\" B=\"152\"/>\n"
                                                    "    <COLOR ID=\"806\" R=\"255\" G=\"0\" B=\"0\"/>\n"
                                                    "    <COLOR ID=\"807\" R=\"0\" G=\"0\" B=\"0\"/>\n"
                                                    "  </PROCESSORLIST>\n"
                                                    "  <UICOMPONENT isProcessorListOpen=\"1\" isEditorViewportOpen=\"1\"/>\n"
                                                    "  <AUDIO sampleRate=\"44100.0\" bufferSize=\"1024\" deviceType=\"CoreAudio\"/>\n"
                                                    "</SETTINGS>";

    XmlDocument doc (docText);
    std::unique_ptr<XmlElement> xml = doc.getDocumentElement();
    ASSERT_TRUE (xml);
    ASSERT_TRUE (xml->hasTagName ("SETTINGS"));

    processorGraph->loadFromXml (xml.get());
    auto processors = processorGraph->getListOfProcessors();
    ASSERT_EQ (processors.size(), 2);

    GenericProcessor* bandpassFilter = nullptr;
    GenericProcessor* fileReader = nullptr;
    for (const auto processor : processors)
    {
        if (processor->getName() == "Bandpass Filter")
        {
            bandpassFilter = processor;
        }
        if (processor->getName() == "File Reader")
        {
            fileReader = processor;
        }
    }

    ASSERT_TRUE (bandpassFilter != nullptr);
    ASSERT_TRUE (fileReader != nullptr);

    ASSERT_EQ (fileReader->getSourceNode(), nullptr);
    ASSERT_EQ (fileReader->getDestNode(), bandpassFilter);
    ASSERT_EQ (bandpassFilter->getSourceNode(), fileReader);
    ASSERT_EQ (bandpassFilter->getDestNode(), nullptr);
}
