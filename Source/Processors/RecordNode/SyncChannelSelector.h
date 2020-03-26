#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"

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

class SyncChannelSelector : public Component, public Button::Listener
{
public:
	SyncChannelSelector(int nChans, int selectedChannelIdx);
	~SyncChannelSelector();

	void mouseDown(const MouseEvent &event);
	void mouseMove(const MouseEvent &event);
	void mouseUp(const MouseEvent &event);
	void buttonClicked(Button*);

	int nChannels;
	int selectedId;

	OwnedArray<SyncChannelButton> buttons;

private:

    ScopedPointer<SetButton> setMasterSubprocessorButton;
    
};
