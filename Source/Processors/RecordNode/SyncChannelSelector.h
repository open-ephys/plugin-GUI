#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../../Utils/Utils.h"

class SyncChannelSelector;

class SyncChannelButton : public Button	
{
public:
	SyncChannelButton(int id, SyncChannelSelector* parent);
	~SyncChannelButton();
    int getId() { return id; };
private:
	//void mouseDown(const MouseEvent &event);
	//void mouseUp(const MouseEvent &event);

	int id; 
	SyncChannelSelector* parent;
    int width;
    int height;
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class SetButton : public Button	
{
public:
	SetButton(const String& name);
	~SetButton();
private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class SyncControlButton;

class SyncChannelSelector : public Component, public Button::Listener
{
public:
	SyncChannelSelector(int nChans, int selectedChannelIdx, bool isMaster);
	~SyncChannelSelector();

	void mouseDown(const MouseEvent &event);
	void mouseMove(const MouseEvent &event);
	void mouseUp(const MouseEvent &event);
	void buttonClicked(Button*);

	int nChannels;
	int selectedId;
	bool isMaster;

	int buttonSize;
	int nRows;

	int width;
	int height;

	OwnedArray<SyncChannelButton> buttons;

private:

    ScopedPointer<SetButton> setMasterSubprocessorButton;
    
};
