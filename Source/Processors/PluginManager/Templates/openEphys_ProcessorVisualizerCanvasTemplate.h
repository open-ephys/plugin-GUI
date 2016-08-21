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

#ifndef HEADERGUARD
#define HEADERGUARD

#include <VisualizerEditorHeaders.h>
#include <AllLookAndFeels.h>

#include "PROCESSORCLASSNAME.h"
#include "CONTENTCOMPONENTCLASSNAME.h"

/**
    Class for displaying data in any subclasses of VisualizerEditor either in the tab or separate window.

    @see Visualizer, LfpDisplayCanvas, SpikeDisplayCanvas
*/
class EDITORCANVASCLASSNAME : public Visualizer
{
public:
    /** The class constructor, used to initialize any members. */
    EDITORCANVASCLASSNAME (PROCESSORCLASSNAME* processor);

    /** The class destructor, used to deallocate memory */
    ~EDITORCANVASCLASSNAME();

    /** Called every time when canvas is resized or moved. */
    void resized() override;

    /** Called when the component's tab becomes visible again.*/
    void refreshState() override;

    /** Called when parameters of underlying data processor are changed.*/
    void update() override;

    /** Called instead of "repaint" to avoid redrawing underlying components if not necessary.*/
    void refresh() override;

    /** Called when data acquisition is active.*/
    void beginAnimation() override;

    /** Called when data acquisition ends.*/
    void endAnimation() override;

    /** Called by an editor to initiate a parameter change.*/
    void setParameter (int, float) override;

    /** Called by an editor to initiate a parameter change.*/
    void setParameter (int, int, int, float) override;

private:
    PROCESSORCLASSNAME* processor;

    // This component contains all components and graphics that were added using Projucer.
    // It's bounds initially have same bounds as the canvas itself.
    CONTENTCOMPONENTCLASSNAME content;
    //
    //ScopedPointer<LookAndFeel> m_contentLookAndFeel;

    // ========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EDITORCANVASCLASSNAME);
};


#endif // HEADERGUARD
