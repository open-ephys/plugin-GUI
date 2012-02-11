
//   ==============================================================================

//     Visualizer.h
//     Created: 15 Jul 2011 8:42:01pm
//     Author:  jsiegle

//   ==============================================================================


// #ifndef __VISUALIZER_H_5573CACE__
// #define __VISUALIZER_H_5573CACE__


// #include "../../../JuceLibraryCode/JuceHeader.h"
// #include "GenericEditor.h"
// #include "../../UI/UIComponent.h"
// #include "../../UI/DataViewport.h"

// #ifdef _WIN32
// #include <windows.h>
// #endif

// #if JUCE_WINDOWS
// #include <gl/gl.h>
// #include <gl/glu.h>
// #elif JUCE_LINUX
// #include <GL/gl.h>
// #include <GL/glut.h>
// #undef KeyPress
// #elif JUCE_IPHONE
// #include <OpenGLES/ES1/gl.h>
// #include <OpenGLES/ES1/glext.h>
// #elif JUCE_MAC
// #include <GLUT/glut.h>
// #endif

// #ifndef GL_BGRA_EXT
// #define GL_BGRA_EXT 0x80e1
// #endif


// class FilterViewport;
// class DataViewport;

// class DataWindow : public DocumentWindow
// {
// public:
// 	DataWindow(Button* button);
// 	~DataWindow();

// 	void closeButtonPressed();

// private:
// 	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataWindow);

// 	Button* controlButton;

// };

// class SelectorButton : public DrawableButton
// {
// 	public:
// 		SelectorButton();
// 		~SelectorButton();	
// };

// class Visualizer : public GenericEditor,
// 				   public Button::Listener
// {
// public:
// 	Visualizer (GenericProcessor*, FilterViewport*, DataViewport*);
// 	~Visualizer();

// 	void buttonClicked (Button* button);
// 	void setBuffers (AudioSampleBuffer*, MidiBuffer*);
// 	void setUIComponent (UIComponent* ui) {UI = ui;}

// private:	
// 	//Slider* slider;
// 	ScopedPointer <DataWindow> dataWindow;

// 	SelectorButton* windowSelector;
// 	SelectorButton* tabSelector;

// 	AudioSampleBuffer* streamBuffer;
// 	MidiBuffer* eventBuffer;
// 	UIComponent* UI;
// 	DataViewport* dataViewport;

// 	int tabIndex;

// 	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualizer);

// };

// class Renderer : public OpenGLComponent,
// 				 public ActionListener

// {
// public:
// 	Renderer(AudioSampleBuffer* streamBuffer, MidiBuffer* eventBuffer, UIComponent* ui);
// 	~Renderer();
// 	virtual void newOpenGLContextCreated() = 0;
// 	virtual void renderOpenGL() = 0;

// 	AudioSampleBuffer* streamBuffer;
// 	MidiBuffer* eventBuffer;

// 	Configuration* config;

// private:

// 	void actionListenerCallback(const String& msg);


// };




// #endif  // __VISUALIZER_H_5573CACE__
