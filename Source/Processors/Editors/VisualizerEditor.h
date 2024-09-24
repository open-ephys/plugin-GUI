/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#ifndef __VISUALIZEREDITOR_H_17E6D78C__
#define __VISUALIZEREDITOR_H_17E6D78C__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Visualization/DataWindow.h"
#include "../Visualization/Visualizer.h"
#include "GenericEditor.h"

class DataWindow;
class Visualizer;

/**
    Button for selecting the location of a visualizer.
    (either in a tab or a separate window)

    @see VisualizerEditor
*/
class PLUGIN_API SelectorButton : public Button
{
public:
    /** Constructor */
    SelectorButton (const String& buttonName);

    /** Destructor */
    ~SelectorButton() {}

private:
    /** Renders the button*/
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    /** Returns true if it's a window button*/
    bool isOpenWindowButton() const;

    /** Returns true if it's a tab button*/
    bool isOpenTabButton() const;
};

/**
    Base class for creating editors with visualizers (large graphical displays
    that appear in a tab or a separate window).

    Automatically adds buttons (and their handlers) which open the canvas in a window or
    a tab.

    @see GenericEditor, Visualizer

*/
class PLUGIN_API VisualizerEditor : public GenericEditor, public DataWindow::Listener
{
public:
    /** Constructor 
    *   Sets the text that will appear in the Visualizer's tab or window
    *   Optionally defines the desired width of the editor
    */
    VisualizerEditor (GenericProcessor* processor, String tabText, int desiredWidth = 180);

    /** Destructor -- closes the tab if it's still open */
    ~VisualizerEditor();

    // ------------------------------------------------------------
    //                  PURE VIRTUAL METHOD
    //     (must be implemented by all VisualizerEditors)
    // ------------------------------------------------------------

    /** Creates a new Visualizer canvas. This is like a factory method and must be defined in your sub-class. */
    virtual Visualizer* createNewCanvas() = 0;

    // ------------------------------------------------------------
    //                   VIRTUAL METHODS
    //       (can optionally be overriden by sub-classes)
    // ------------------------------------------------------------

    /** Use this method to save custom editor parameters */
    virtual void saveVisualizerEditorParameters (XmlElement* xml) {}

    /** Use this method to load custom editor parameters */
    virtual void loadVisualizerEditorParameters (XmlElement* xml) {}

    /** Called when the Visualizer window is closed */
    virtual void windowClosed() override {}

    /** Called after tab has been closed. */
    void tabWasClosed();

    /** Adds a new tab to the DataViewport. */
    void addTab();

    /** Calls Visualizer's beginAnimation() method */
    virtual void enable();

    /** Calls Visualizer's endAnimation() method */
    virtual void disable();

    /** Returns true if it is a VisualizerEditor */
    bool isVisualizerEditor() override final { return true; }

    // ------------------------------------------------------------
    //                     OTHER METHODS
    // ------------------------------------------------------------

    /** Returns a pointer to the visualizer (used by the DataViewport) */
    Component* getVisualizerComponent();

    /** Sets the location of the window + tab buttons*/
    void resized() override;

    /** Brings the Visualizer to the foreground  */
    void editorWasClicked() override;

    /** Calls the Visualizer's update() method */
    void updateVisualizer() override;

    /** Saves Visualizer open/closed state to XML */
    void saveCustomParametersToXml (XmlElement* xml) override;

    /** Loads Visualizer open/closed state from XML */
    void loadCustomParametersFromXml (XmlElement* xml) override;

    std::unique_ptr<DataWindow> dataWindow;
    std::unique_ptr<Visualizer> canvas;

    /** The text shown in this visualizer's tab*/
    String tabText;

protected:
    /**
        @brief      Creates a new DataWindow using the windowSelector (button)
                    and ``tabText``. The new object is stored in (and owned by)
                    VisualizerEditor::dataWindow.
        @details    Use this to make a new DataWindow. If needed, you can
                    transfer ownership of the new object from
                    VisualizerEditor::dataWindow to _your_ own ScopedPointer.
        @note       This method provides an interface to DataWindow, DataWindow
                    methods cannot be defined in derivations (ie, plugins).
    */
    void makeNewWindow();

    /**
        @brief      Adds a closeWindow listener for dw.

        @note       This method provides an interface to DataWindow, DataWindow
                    methods cannot be defined in derivations (ie, plugins).
    */
    static void addWindowListener (DataWindow* window, DataWindow::Listener* newListener);

    /**
        @brief      Removes a closeWindow listener for dw.

        @note       This method provides an interface to DataWindow, DataWindow
                    methods cannot be defined in derivations (ie, plugins).
    */
    static void removeWindowListener (DataWindow* window, DataWindow::Listener* oldListener);

    /**
        @brief      Use this to efficiently compare or find what is on the
                    currently active tab.

        @return     The active tab content Component.
    */
    Component* getActiveTabContentComponent() const;

    /** Removes the tab from the DataViewport. */
    void removeTab();

    /** Checks and creates a canvas if one doesn't exist. Also, updates the canvas */
    void checkForCanvas();

    /** True if tab is currently active*/
    bool isOpenInTab = false;

    /** So that we can override buttonClick. That's not possible if these are private. */
    std::unique_ptr<SelectorButton> windowSelector;
    std::unique_ptr<SelectorButton> tabSelector;

private:
    /** Custom class for responding to button clicks */
    class ButtonResponder : public Button::Listener
    {
    public:
        ButtonResponder (VisualizerEditor* editor_) : editor (editor_) {}
        void buttonClicked (Button* button);

    private:
        VisualizerEditor* editor;
    };

    ButtonResponder dataWindowButtonListener;

    void initializeSelectors();

    // Some constants

    //C++11 constexpr keyword is not implemented in Visual Studio prior 2015
#if defined _MSC_VER && _MSC_VER <= 1800
    const char* EDITOR_TAG_TAB = "TAB";
    const char* EDITOR_TAG_WINDOW = "WINDOW";
#else
    static constexpr const char* EDITOR_TAG_TAB = "TAB";
    static constexpr const char* EDITOR_TAG_WINDOW = "WINDOW";
#endif

    // ========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualizerEditor);
};

#endif // __VISUALIZEREDITOR_H_17E6D78C__
