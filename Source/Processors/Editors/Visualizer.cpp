
//   ==============================================================================

//     Visualizer.cpp
//     Created: 15 Jul 2011 8:42:01pm
//     Author:  jsiegle

//   ==============================================================================


// #include "Visualizer.h"
// #include "../Visualization/SpikeViewer.h"
// #include "../Visualization/LfpViewer.h"

// SelectorButton::SelectorButton()
// 	: DrawableButton (T("Selector"), DrawableButton::ImageFitted)
// {
// 	DrawablePath normal, over, down;

// 	    Path p;
//         p.addEllipse (0.0,0.0,20.0,20.0);
//         normal.setPath (p);
//         normal.setFill (Colours::lightgrey);
//         normal.setStrokeThickness (0.0f);

//         over.setPath (p);
//         over.setFill (Colours::black);
//         over.setStrokeFill (Colours::black);
//         over.setStrokeThickness (5.0f);

//         setImages (&normal, &over, &over);
//         setBackgroundColours(Colours::darkgrey, Colours::green);
//         setClickingTogglesState (true);
//         setTooltip ("Toggle a state.");

// }

// SelectorButton::~SelectorButton()
// {
// }


// Visualizer::Visualizer (GenericProcessor* parentNode, 
// 						FilterViewport* vp,
// 						DataViewport* dv) 
// 	: GenericEditor(parentNode, vp), dataViewport(dv),
// 	  tabIndex(-1), dataWindow(0),
// 	  streamBuffer(0), eventBuffer(0)

// {
// 	desiredWidth = 210;

// 	windowSelector = new SelectorButton();
// 	windowSelector->addListener(this);
// 	windowSelector->setBounds(25,25,20,20);
// 	windowSelector->setToggleState(false,false);
// 	addAndMakeVisible(windowSelector);

// 	tabSelector = new SelectorButton();
// 	tabSelector->addListener(this);
// 	tabSelector->setBounds(25,50,20,20);
// 	tabSelector->setToggleState(false,false);
// 	addAndMakeVisible(tabSelector);

// }

// Visualizer::~Visualizer()
// {

// 	if (tabIndex > -1)
// 	{
// 		dataViewport->removeTab(tabIndex);
// 	}

// 	deleteAllChildren();

// }

// void Visualizer::setBuffers(AudioSampleBuffer* asb, MidiBuffer* mb)
// {
// 	std::cout << "Buffers are set!" << std::endl;
// 	streamBuffer = asb;
// 	eventBuffer = mb;


// 	std::cout << streamBuffer << std::endl;
// 	std::cout << eventBuffer << std::endl;
// }

// void Visualizer::buttonClicked(Button* button)
// {
// 	if (button == windowSelector)
// 	{
// 		if (dataWindow == 0) {
// 			dataWindow = new DataWindow(windowSelector);

// 			if (getName().equalsIgnoreCase("Visualizers/LFP Viewer"))
// 				dataWindow->setContentComponent(new LfpViewer(streamBuffer,eventBuffer,UI));
// 			else if (getName().equalsIgnoreCase("Visualizers/Spike Viewer"))
// 				dataWindow->setContentComponent(new SpikeViewer(streamBuffer,eventBuffer,UI));

// 			dataWindow->setVisible(true);
			
// 		} else {
// 			dataWindow->setVisible(windowSelector->getToggleState());
// 		}

// 	} else if (button == tabSelector)
// 	{
// 		if (tabSelector->getToggleState() && tabIndex < 0)
// 		{
// 			if (getName().equalsIgnoreCase("Visualizers/LFP Viewer"))
// 				tabIndex = dataViewport->addTabToDataViewport("LFP",new LfpViewer(streamBuffer,eventBuffer,UI));
// 			else if (getName().equalsIgnoreCase("Visualizers/Spike Viewer"))
// 				tabIndex = dataViewport->addTabToDataViewport("Spikes",new SpikeViewer(streamBuffer,eventBuffer,UI));
		
// 		} else if (!tabSelector->getToggleState() && tabIndex > -1)
// 		{
// 			dataViewport->removeTab(tabIndex);
// 			tabIndex = -1;
// 		}
// 	}
// }

// //===================================================

// Renderer::Renderer(AudioSampleBuffer* sBuffer, MidiBuffer* eBuffer, UIComponent* ui)
// 	: streamBuffer(sBuffer), eventBuffer(eBuffer)
// {
// 	//ui->addActionListener(this);
// 	config = ui->getConfiguration();
// }

// Renderer::~Renderer() { }

// void Renderer::actionListenerCallback(const String & msg)
// {
// 	repaint();
// }


// DataWindow::DataWindow(Button* cButton)
// 	: DocumentWindow ("Stream Window", 
// 					  Colours::black, 
// 					  DocumentWindow::allButtons),
// 	  controlButton(cButton)

// {
// 	centreWithSize(300,200);
// 	setUsingNativeTitleBar(true);
// 	setResizable(true,true);
// 	setTitleBarHeight(40);
// }

// DataWindow::~DataWindow()
// {
// 	//deleteAllChildren();
// 	setContentComponent (0);
	
// }

// void DataWindow::closeButtonPressed()
// {
// 	setVisible(false);
// 	controlButton->setToggleState(false,false);
// 	//viewport->removeTab(0);

// }