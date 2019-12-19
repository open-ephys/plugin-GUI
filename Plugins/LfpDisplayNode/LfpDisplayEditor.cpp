/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "LfpDisplayEditor.h"

using namespace LfpViewer;


LfpDisplayEditor::LfpDisplayEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors)
, hasNoInputs(true)
{
    lfpProcessor = (LfpDisplayNode*) parentNode;
    tabText = "LFP";

    desiredWidth = 180;
    
    subprocessorSelectionLabel = new Label("Display subprocessor sample rate", "Display Subprocessor:");
    subprocessorSelectionLabel->setBounds(10, 30, 130, 20);
    addAndMakeVisible(subprocessorSelectionLabel);

    subprocessorSelection = new ComboBox("Subprocessor sample rate");
    subprocessorSelection->setBounds(10, 55, 130, 22);
    subprocessorSelection->addListener(this);
    addAndMakeVisible(subprocessorSelection);
    
    subprocessorSampleRateLabel = new Label("Subprocessor sample rate label", "Sample Rate:");
    subprocessorSampleRateLabel->setFont(Font(Font::getDefaultSerifFontName(), 14, Font::plain));
    subprocessorSampleRateLabel->setBounds(subprocessorSelection->getX(), subprocessorSelection->getBottom() + 10, 200, 40);
    addAndMakeVisible(subprocessorSampleRateLabel);

	defaultSubprocessor = 0;
}

LfpDisplayEditor::~LfpDisplayEditor()
{
}

void LfpDisplayEditor::startAcquisition()
{
	//subprocessorSelection->setEnabled(false);
}

void LfpDisplayEditor::stopAcquisition()
{
	//subprocessorSelection->setEnabled(true);
}

Visualizer* LfpDisplayEditor::createNewCanvas()
{
    canvas = new LfpDisplayCanvas(lfpProcessor);
    return canvas;
}

void LfpDisplayEditor::buttonClicked(Button *button)
{
    // duplicate default VisualizerEditor behavior, except...
    if (canvas == nullptr)
    {
        canvas = createNewCanvas();
        
        // initialize the subprocessor sample rate filtering before canvas updates
        // (else) initialization errors. lots of time-critical cross dependencies here,
        // should be cleaned up
        updateSubprocessorSelectorOptions();
        
        canvas->update();
        
        if (isPlaying)
            canvas->beginAnimation();
    }
    
    // resume default behavior
    VisualizerEditor::buttonClicked(button);
}

// not really being used (yet)...
void LfpDisplayEditor::buttonEvent(Button* button)
{


}

void LfpDisplayEditor::comboBoxChanged(juce::ComboBox *cb)
{
    if (cb == subprocessorSelection)
    {
        std::cout << "Setting subprocessor to " << cb->getSelectedId() << std::endl;
        uint32 subproc = inputSubprocessors[cb->getSelectedId() - 1];
		
        String sampleRateLabelText = "Sample Rate: ";
		sampleRateLabelText += String(lfpProcessor->getSubprocessorSampleRate(subproc));
		subprocessorSampleRateLabel->setText(sampleRateLabelText, dontSendNotification);
        std::cout << sampleRateLabelText << std::endl;

        lfpProcessor->setSubprocessor(subproc);
        if (canvas)
        {
            static_cast<LfpDisplayCanvas*>(canvas.get())->setDrawableSubprocessor(subproc);
        }
    }
}

void LfpDisplayEditor::updateSubprocessorSelectorOptions()
{
    // clear out the old data
    inputSubprocessors.clear();
    subprocessorSelection->clear(dontSendNotification);
    
	if (lfpProcessor->getTotalDataChannels() != 0)

	{
        HashMap<int, String> subprocessorNames;

		for (int i = 0, len = lfpProcessor->getTotalDataChannels(); i < len; ++i)
		{
            const DataChannel* ch = lfpProcessor->getDataChannel(i);
            uint16 sourceNodeId = ch->getSourceNodeID();
			uint16 subProcessorIdx = ch->getSubProcessorIdx();
            uint32 subProcFullId = GenericProcessor::getProcessorFullId(sourceNodeId, subProcessorIdx);

			bool added = inputSubprocessors.add(subProcFullId);

            if (added)
            {
                String sourceName = ch->getSourceName();
                subprocessorNames.set(subProcFullId,
                    sourceName + " " + String(sourceNodeId) + "/" + String(subProcessorIdx));
            }
		}

		for (int i = 0; i < inputSubprocessors.size(); ++i)
		{
			subprocessorSelection->addItem(subprocessorNames[inputSubprocessors[i]], i + 1);
		}

        uint32 selectedSubproc = lfpProcessor->getSubprocessor();
        int selectedSubprocId = (selectedSubproc ? inputSubprocessors.indexOf(selectedSubproc) : defaultSubprocessor) + 1;

		subprocessorSelection->setSelectedId(selectedSubprocId, sendNotification);
	}
    else
    {
        subprocessorSelection->addItem("None", 1);
        subprocessorSelection->setSelectedId(1, dontSendNotification);

        String sampleRateLabelText = "Sample Rate: <not available>";
        subprocessorSampleRateLabel->setText(sampleRateLabelText, dontSendNotification);
        //setCanvasDrawableSubprocessor(-1);
    }
}

SortedSet<uint32> LfpDisplayEditor::getInputSubprocessors()
{
	return inputSubprocessors;
}

void LfpDisplayEditor::saveVisualizerParameters(XmlElement* xml)
{

	xml->setAttribute("Type", "LfpDisplayEditor");

	int subprocessorItemId = subprocessorSelection->getSelectedId();

	XmlElement* values = xml->createNewChildElement("VALUES");
	values->setAttribute("SubprocessorId", subprocessorItemId - 1);
}

void LfpDisplayEditor::loadVisualizerParameters(XmlElement* xml)
{

	forEachXmlChildElement(*xml, xmlNode)
	{
		if (xmlNode->hasTagName("VALUES"))
		{
			std::cout << "LfpDisplay found " << xmlNode->getIntAttribute("SubprocessorId") << std::endl;
			defaultSubprocessor = xmlNode->getIntAttribute("SubprocessorId");
			subprocessorSelection->setSelectedItemIndex(defaultSubprocessor, sendNotification);

		}
	}

}