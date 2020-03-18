#include "EphysSocketEditor.h"
#include "EphysSocket.h"

#include <string>
#include <iostream>

using namespace EphysSocketNode;

EphysSocketEditor::EphysSocketEditor(GenericProcessor* parentNode, EphysSocket *socket) : GenericEditor(parentNode, false)
{
    node = socket;

    desiredWidth = 240;

    // Add connect button
    connectButton = new UtilityButton("CONNECT", Font("Small Text", 13, Font::bold));
    connectButton->setRadius(3.0f);
    connectButton->setBounds(5, 35, 65, 20);
    connectButton->addListener(this);
    addAndMakeVisible(connectButton);

    // Port
    portLabel = new Label("Port", "Port");
    portLabel->setFont(Font("Small Text", 10, Font::plain));
    portLabel->setBounds(5, 60, 65, 8);
    portLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(portLabel);

    portText = new TextEditor("Port");
    portText->setFont(Font("Small Text", 10, Font::plain));
    portText->setText(std::to_string(node->port));
    portText->setBounds(5, 70, 65, 15);
    addAndMakeVisible(portText);

    //---

    // Num chans
    chanLabel = new Label("Num. Ch.", "Num. Ch.");
    chanLabel->setFont(Font("Small Text", 10, Font::plain));
    chanLabel->setBounds(80, 30, 65, 8);
    chanLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(chanLabel);

    chanText = new TextEditor("Num. Ch.");
    chanText->setFont(Font("Small Text", 10, Font::plain));
    chanText->setText(std::to_string(node->num_channels));
    chanText->setBounds(80, 40, 65, 15);
    addAndMakeVisible(chanText);

    // Num samples
    sampLabel = new Label("Num. Samp.", "Num. Samp.");
    sampLabel->setFont(Font("Small Text", 10, Font::plain));
    sampLabel->setBounds(80, 60, 65, 8);
    sampLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(sampLabel);

    sampText = new TextEditor("Num. Samp.");
    sampText->setFont(Font("Small Text", 10, Font::plain));
    sampText->setText(std::to_string(node->num_samp));
    sampText->setBounds(80, 70, 65, 15);
    addAndMakeVisible(sampText);

    // Fs
    fsLabel = new Label("Fs (Hz)", "Fs (Hz)");
    fsLabel->setFont(Font("Small Text", 10, Font::plain));
    fsLabel->setBounds(80, 90, 65, 8);
    fsLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(fsLabel);

    fsText = new TextEditor("Fs (Hz)");
    fsText->setFont(Font("Small Text", 10, Font::plain));
    fsText->setText(std::to_string(node->sample_rate));
    fsText->setBounds(80, 100, 65, 15);
    addAndMakeVisible(fsText);

    //---

    // Scale
    scaleLabel = new Label("Scale", "Scale");
    scaleLabel->setFont(Font("Small Text", 10, Font::plain));
    scaleLabel->setBounds(155, 30, 65, 8);
    scaleLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(scaleLabel);

    scaleText = new TextEditor("Scale");
    scaleText->setFont(Font("Small Text", 10, Font::plain));
    scaleText->setText(std::to_string(node->data_scale));
    scaleText->setBounds(155, 40, 65, 15);
    addAndMakeVisible(scaleText);

    // Offset
    offsetLabel = new Label("Offset", "Offset");
    offsetLabel->setFont(Font("Small Text", 10, Font::plain));
    offsetLabel->setBounds(155, 60, 65, 8);
    offsetLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(offsetLabel);

    offsetText = new TextEditor("Offset");
    offsetText->setFont(Font("Small Text", 10, Font::plain));
    offsetText->setText(std::to_string(node->data_offset));
    offsetText->setBounds(155, 70, 65, 15);
    addAndMakeVisible(offsetText);

    transposeButton.setBounds(155, 95, 65, 20);
    transposeButton.setClickingTogglesState(true);
    addAndMakeVisible(transposeButton);
}

void EphysSocketEditor::startAcquisition()
{
    // Disable the whole gui
    portText->setEnabled(false);
    chanText->setEnabled(false);
    sampText->setEnabled(false);
    fsText->setEnabled(false);
    scaleText->setEnabled(false);
    offsetText->setEnabled(false);
    connectButton->setEnabled(false);
    transposeButton.setEnabled(false);

    // Set the channels etc
    node->num_channels = chanText->getText().getIntValue();
    node->num_samp = sampText->getText().getIntValue();
    node->sample_rate = fsText->getText().getFloatValue();
    node->data_scale = scaleText->getText().getFloatValue();
    node->data_offset = offsetText->getText().getIntValue();
    node->transpose = transposeButton.getToggleState();

    node->resizeChanSamp();
}

void EphysSocketEditor::stopAcquisition()
{
    // Reenable the whole gui
    portText->setEnabled(true);
    chanText->setEnabled(true);
    sampText->setEnabled(true);
    fsText->setEnabled(true);
    scaleText->setEnabled(true);
    offsetText->setEnabled(true);
    connectButton->setEnabled(true);
    transposeButton.setEnabled(true);
}

void EphysSocketEditor::buttonEvent(Button* button)
{
    // Only one button
    node->port = portText->getText().getIntValue();
    node->tryToConnect();
}

void EphysSocketEditor::saveEditorParameters(XmlElement* xmlNode)
{
    XmlElement* parameters = xmlNode->createNewChildElement("PARAMETERS");

    parameters->setAttribute("port", portText->getText());
    parameters->setAttribute("numchan", chanText->getText());
    parameters->setAttribute("numsamp", sampText->getText());
    parameters->setAttribute("fs", fsText->getText());
    parameters->setAttribute("scale", scaleText->getText());
    parameters->setAttribute("offset", offsetText->getText());
}

void EphysSocketEditor::loadEditorParameters(XmlElement* xmlNode)
{
    forEachXmlChildElement(*xmlNode, subNode)
    {
        if (subNode->hasTagName("PARAMETERS"))
        {
            portText->setText(subNode->getStringAttribute("port", ""));
            chanText->setText(subNode->getStringAttribute("numchan", ""));
            sampText->setText(subNode->getStringAttribute("numsamp", ""));
            fsText->setText(subNode->getStringAttribute("fs", ""));
            scaleText->setText(subNode->getStringAttribute("scale", ""));
            offsetText->setText(subNode->getStringAttribute("offset", ""));
        }
    }
}

