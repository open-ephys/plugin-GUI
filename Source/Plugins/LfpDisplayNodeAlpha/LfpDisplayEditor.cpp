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

using namespace LfpDisplayNodeAlpha;


LfpDisplayEditor::LfpDisplayEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors)

{
    lfpProcessor = (LfpDisplayNode*) parentNode;
    tabText = "LFP";

    desiredWidth = 180;
    
    subprocessorSelection = new ComboBox("Subprocessor sample rate");
//    subprocessorSelection->setBounds(subprocessorSelectionLabel->getX()+5, subprocessorSelectionLabel->getBottom(), 60, 22);
    subprocessorSelection->setBounds(10, 30, 50, 22);
    subprocessorSelection->addListener(this);
    addAndMakeVisible(subprocessorSelection);
    
    subprocessorSelectionLabel = new Label("Display subprocessor sample rate", "Display Subproc.");
    //    subprocessorSelectionLabel->setBounds(10, 25, 140, 20);
    subprocessorSelectionLabel->setBounds(subprocessorSelection->getRight(), subprocessorSelection->getY(), 100, 20);
    addAndMakeVisible(subprocessorSelectionLabel);
    
    subprocessorSampleRateLabel = new Label("Subprocessor sample rate label", "Sample Rate:");
    subprocessorSampleRateLabel->setFont(Font(Font::getDefaultSerifFontName(), 14, italic));
    subprocessorSampleRateLabel->setBounds(subprocessorSelection->getX(), subprocessorSelection->getBottom() + 10, 200, 40);
    addAndMakeVisible(subprocessorSampleRateLabel);
}

LfpDisplayEditor::~LfpDisplayEditor()
{
}

Visualizer* LfpDisplayEditor::createNewCanvas()
{
    canvas = new LfpDisplayCanvas(lfpProcessor);
    updateSubprocessorSelectorOptions();
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
        ((LfpDisplayCanvas *)canvas.get())->setDrawableSubprocessor(*(inputSubprocessorIndices.begin() + (subprocessorSelection->getSelectedId() - 1)));
//        ((LfpDisplayCanvas*)canvas.get())->setDrawableSampleRate(*(inputSampleRates.begin() + (subprocessorSelection->getSelectedId() - 1)));
        
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
//        setCanvasDrawableSampleRate(cb->getSelectedId() - 1);
        setCanvasDrawableSubprocessor(cb->getSelectedId() - 1);
    }
}

void LfpDisplayEditor::updateSubprocessorSelectorOptions()
{
    // clear out the old data
    inputSubprocessorIndices.clear();
    inputSampleRates.clear();
    subprocessorSelection->clear(dontSendNotification);
    
    for (int i = 0, len = lfpProcessor->getTotalDataChannels(); i < len; ++i)
    {
        int subProcessorIdx = lfpProcessor->getDataChannel(i)->getSubProcessorIdx();
        
        bool success = inputSubprocessorIndices.add(subProcessorIdx);
        
        if (success) inputSampleRates.set(subProcessorIdx, lfpProcessor->getDataChannel(i)->getSampleRate());
        
//        if (success) std::cout << "\t\tadding subprocessor index " << subProcessorIdx << std::endl;
    }
    
    int subprocessorToSet = -1;
    if (inputSubprocessorIndices.size() > 0)
    {
        subprocessorToSet = 0;
    }
    
    for (int i = 0; i < inputSubprocessorIndices.size(); ++i)
    {
        subprocessorSelection->addItem (String (*(inputSubprocessorIndices.begin() + i)), i + 1);
    }
    
    if (subprocessorToSet >= 0)
    {
        subprocessorSelection->setSelectedId(subprocessorToSet + 1, dontSendNotification);
        
        String sampleRateLabelText = "Sample Rate: ";
        sampleRateLabelText += String(inputSampleRates[*(inputSubprocessorIndices.begin()+subprocessorToSet)]);
        
        subprocessorSampleRateLabel->setText(sampleRateLabelText, dontSendNotification);
        setCanvasDrawableSubprocessor(subprocessorToSet);
    }
}

void LfpDisplayEditor::setCanvasDrawableSubprocessor(int index)
{
    if (canvas)
    {
        ((LfpDisplayCanvas *)canvas.get())->setDrawableSubprocessor(*(inputSubprocessorIndices.begin() + index));
    }
}

