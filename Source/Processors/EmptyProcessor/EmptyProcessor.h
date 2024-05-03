/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifndef EMPTYPROCESSOR_H_INCLUDED
#define EMPTYPROCESSOR_H_INCLUDED

#include <JuceHeader.h>
#include "../GenericProcessor/GenericProcessor.h"

/** 
* Creates an empty processor in signal chains without any source.
* 
* Data acquisition cannot begin while a EmptyProcessor is in the signal chain.

*/
class EmptyProcessor : public GenericProcessor
{
public:

    /** Constructor*/
    EmptyProcessor ();

    /** Destructor*/
    ~EmptyProcessor();

    /** Creates a Empty editor.*/
    AudioProcessorEditor* createEditor() override;

    /** Returns false, to prevent acquisition from starting.*/
    bool startAcquisition() override;

    /** Empty method */
    void process (AudioBuffer<float>& continuousBuffer) override { }

    /** Set isEnabled to 'false', since this plugin will not work in the signal chain*/
    void updateSettings() override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EmptyProcessor);
};



#endif  // EMPTYPROCESSOR_H_INCLUDED
