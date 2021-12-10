#ifndef DEFAULTCONFIG_H_INCLUDED
#define DEFAULTCONFIG_H_INCLUDED

#include "../../JuceLibraryCode/JuceHeader.h"

class MainWindow;

class DefaultConfigComponent : 
	public Component,
    public Button::Listener
{
public:
	DefaultConfigComponent();
	~DefaultConfigComponent();

	void paint(Graphics& g) override;

    void resized() override;

    void buttonClicked(Button*) override;

private:
	std::unique_ptr<Label> configLabel;
    std::unique_ptr<ComboBox> configSelector;
    std::unique_ptr<TextButton> goButton;

    std::unique_ptr<ImageButton> acqBoardButton;
    std::unique_ptr<Label> acqBoardLabel;

    std::unique_ptr<ImageButton> fileReaderButton;
    std::unique_ptr<Label> fileReaderLabel;

    std::unique_ptr<ImageButton> neuropixelsButton;
    std::unique_ptr<Label> neuropixelsLabel;

    Font configFont;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DefaultConfigComponent);
};


class DefaultConfigWindow
{
public:
    /** Initializes the DefaultConfigWindow, and sets the window boundaries. */
    DefaultConfigWindow(MainWindow* mainWindow);

    /** Destroys the DefaultConfigWindow. */
    ~DefaultConfigWindow();

    void launchWindow();

private:
    /* Pointer to the main window so we can keep in bounds */
    DocumentWindow* parent;

    DialogWindow* configWindow;

    DefaultConfigComponent* configComponent;

    WeakReference<DefaultConfigWindow>::Master masterReference;
    friend class WeakReference<DefaultConfigWindow>;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DefaultConfigWindow);

};

#endif
