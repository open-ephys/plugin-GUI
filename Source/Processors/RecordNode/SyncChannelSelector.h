#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../../Utils/Utils.h"

class SyncChannelSelector;

/** 

Allows the user to select the TTL line to use for synchronization

*/
class SyncChannelButton : public Button	
{
public:

	/** Constructor */
	SyncChannelButton(int id, SyncChannelSelector* parent);

	/** Destructor */
	~SyncChannelButton();

	/** Returns the ID for this button's stream*/
    int getId() { return id; };

private:

	int id; 
	SyncChannelSelector* parent;
    int width;
    int height;
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class SetButton : public Button	
{
public:

	/** Constructor */
	SetButton(const String& name);

	/** Destructor */
	~SetButton();

private:
	/** Renders the button*/
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};


class SyncChannelSelector : public Component, public Button::Listener
{
public:
	/** Constructor */
	SyncChannelSelector(int nChans, int selectedChannelIdx, bool isPrimary);

	/** Destructor */
	~SyncChannelSelector();

	/** Mouse listener methods*/
	void mouseDown(const MouseEvent &event);
	void mouseMove(const MouseEvent &event);
	void mouseUp(const MouseEvent &event);

	/** Responds to button clicks*/
	void buttonClicked(Button*);

	int nChannels;
	int selectedId;
	bool isPrimary;

	int buttonSize;
	int nRows;

	int width;
	int height;

	OwnedArray<SyncChannelButton> buttons;

private:

    ScopedPointer<SetButton> setPrimaryStreamButton;
    
};
