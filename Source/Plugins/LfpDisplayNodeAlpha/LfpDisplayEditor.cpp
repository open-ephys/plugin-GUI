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
    
    subprocessorSelectionLabel = new Label("Display subprocessor sample rate", "Display Subproc. Sample Rate");
    subprocessorSelectionLabel->setBounds(10, 25, 140, 20);
    addAndMakeVisible(subprocessorSelectionLabel);
    
    subprocessorSelection = new ComboBox("Subprocessor sample rate");
    subprocessorSelection->setBounds(subprocessorSelectionLabel->getX()+5, subprocessorSelectionLabel->getBottom(), 60, 22);
    subprocessorSelection->addListener(this);
    addAndMakeVisible(subprocessorSelection);
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
        ((LfpDisplayCanvas*)canvas.get())->setDrawableSampleRate(*(inputSampleRates.begin() + (subprocessorSelection->getSelectedId() - 1)));
        
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
        setCanvasDrawableSampleRate(cb->getSelectedId() - 1);
    }
}

void LfpDisplayEditor::updateSubprocessorSelectorOptions()
{
    // clear out the old data
    inputSampleRates.clear();
    subprocessorSelection->clear(dontSendNotification);
    
    // get a list of all the sample rates
    for (int i = 0, len = lfpProcessor->getNumInputs(); i < len; ++i)
    {
        float samplerate = lfpProcessor->getDataChannel(i)->getSampleRate();
        bool success = inputSampleRates.add(samplerate);
        
//        if (success) std::cout << "\t\tadding sample rate " << samplerate << " ... size = " << inputSampleRates.size() << std::endl;
    }
    
    // if the source changes, default to first samplerate given
    int sampleRateToSet = -1;
    if (inputSampleRates.size() > 0)
    {
        sampleRateToSet = 0;
    }
    
    // add the samplerate options to the combobox
    for (int i = 0; i < inputSampleRates.size(); ++i)
    {
        subprocessorSelection->addItem(String(*(inputSampleRates.begin()+i)), i+1);
    }
    
    if (sampleRateToSet >= 0)
    {
        setCanvasDrawableSampleRate(sampleRateToSet);
    }
}

void LfpDisplayEditor::setCanvasDrawableSampleRate(int index)
{
    if (canvas)
    {
        std::cout << "selected index = " << index << std::endl;
        ((LfpDisplayCanvas*)canvas.get())->setDrawableSampleRate(*(inputSampleRates.begin() + (index)));
    }
}

