/*
 ------------------------------------------------------------------
 
 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Open Ephys
 
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

#include "GraphViewer.h"
#include "../Processors/Splitter/Splitter.h"
#include "../Processors/Splitter/SplitterEditor.h"
#include "../Processors/Merger/Merger.h"
#include "../Utils/Utils.h"

#include "../Processors/Settings/DataStream.h"
#include "../Processors/Settings/InfoObject.h"

#include "../UI/LookAndFeel/CustomLookAndFeel.h"

const int NODE_WIDTH = 180;
const int NODE_HEIGHT = 50;
const int X_BORDER_SIZE = 45;
const int Y_BORDER_SIZE = 20;

GraphViewport::GraphViewport(GraphViewer* gv)
{
    viewport = std::make_unique<Viewport>();
    viewport->setViewedComponent(gv, false);
    gv->setVisible(true);
    addAndMakeVisible(viewport.get());

    JUCEApplication* app = JUCEApplication::getInstance();
    currentVersionText = "GUI version " + app->getApplicationVersion();

    bw_logo = ImageCache::getFromMemory(BinaryData::bw_logo72_png, BinaryData::bw_logo72_pngSize);

}

void GraphViewport::paint(Graphics& g)
{
    g.fillAll(findColour(ThemeColors::graphViewerBackgroundColorId));
    g.setOpacity(0.6f);
    g.drawImageAt(bw_logo, getWidth() - 175, getHeight() - 115);

    g.setOpacity(1.0f);
    g.setColour(Colours::grey);

    g.setFont(Font("Silkscreen", "Regular", 14));
    g.drawFittedText(currentVersionText, 40, 40, getWidth() - 65, getHeight() - 60, Justification::bottomRight, 100);
}

void GraphViewport::resized()
{
    viewport->setBounds(0, 0, getWidth(), getHeight());
}

GraphViewer::GraphViewer()
{
    setRepaintsOnMouseActivity(false);
    graphViewport = std::make_unique<GraphViewport>(this);
}

void GraphViewer::updateBoundaries()
{
    int maxHeight = 0;
    int maxWidth = graphViewport->getWidth() - 30;

    for (auto node : availableNodes)
    {
        if (node->getBottom() > maxHeight)
            maxHeight = node->getBottom();

        if (node->getRight() > maxWidth)
            maxWidth = node->getRight();
    }

    setBounds(0, 0, maxWidth + 10, maxHeight + 10);
}


void GraphViewer::updateNodes(GenericProcessor* processor, Array<GenericProcessor*> rootProcessors)
{
    // Remove nodes that are not needed anymore
    Array<GraphNode*> nodesToDelete;

    for (auto node : availableNodes)
    {
        if (!node->stillNeeded)
            nodesToDelete.add(node);
    }

    for (auto node : nodesToDelete)
    {
        availableNodes.removeObject(node, true);
    }

    Array<Splitter*> splitters;

    int level = -1;
    levelStartY.clear();

    for (auto processor : rootProcessors)
    {
        int horzShift = -1;
        level++;

        while ((processor != nullptr) || (splitters.size() > 0))
        {
            if (processor != nullptr)
            {
                horzShift++;

                if(processor->isMerger())
                {
                    Merger* merger = (Merger*) processor;
                    if (merger->getSourceNode(0) != nullptr)
                    {
                        GraphNode* nodeA = getNodeForEditor(merger->getSourceNode(0)->getEditor());
                        if(nodeA != nullptr)
                        {
                            level = nodeA->getLevel();
                            horzShift = nodeA->getHorzShift() + 1;
                        }
                    }
                    else if(merger->getSourceNode(1) != nullptr)
                    {
                        level = getNodeForEditor(merger->getSourceNode(1)->getEditor())->getLevel();
                    }
                }

                // Create or update node for processor
                if (!nodeExists(processor))
                {
                    addNode(processor->getEditor(), level, horzShift);
                }
                else // Node exists, updated necessary info
                {
                    GraphNode* node = getNodeForEditor(processor->getEditor());
                    node->stillNeeded = true;
                    node->setLevel(level);
                    node->setHorzShift(horzShift);
                    node->updateStreamInfo();
                    node->updateBoundaries();
                }

                int newY = getNodeForEditor(processor->getEditor())->getBottom() + 35;
                int nextLevel = getNodeForEditor(processor->getEditor())->getLevel() + 1;
                if(levelStartY.count(nextLevel) == 0)
                    levelStartY[nextLevel] = newY;
                else
                    levelStartY[nextLevel] = jmax(levelStartY[nextLevel], newY);

                // Travel down the signal chain
                if (processor->isSplitter())
                {
                    splitters.add((Splitter*) processor);
                    processor = splitters.getLast()->getDestNode(0); // travel down chain 0 first
                } else {
                    processor = processor->getDestNode();
                }
            }
            else // processor is nullptr, look for splitters
            {
                Splitter* splitter = splitters.getFirst();
                processor = splitter->getDestNode(1); // then come back to chain 1

                GraphNode* gn = getNodeForEditor(splitter->getEditor());
                horzShift = gn->getHorzShift();
                
                //check if 2 splitters are connected to 1 splitter
                if(splitter->getDestNode(0) && 
                   splitter->getDestNode(0)->isSplitter() &&
                   processor &&
                   processor->isSplitter())
                {
                    level = gn->getLevel() + 2;
                }
                else
                {
                    level = gn->getLevel() + 1;
                }

                splitters.remove(0);
            }
        }
    }

    updateBoundaries();
    repaint();
    
}

bool GraphViewer::nodeExists(GenericProcessor* p)
{

    if (p != nullptr && getNodeForEditor(p->getEditor()) != nullptr)
        return true;
    
    return false;
}

void GraphViewer::addNode (GenericEditor* editor, int level, int offset)
{
    GraphNode* gn = new GraphNode (editor, this);
    addAndMakeVisible (gn, 0);
    availableNodes.add (gn);
    
    int thisNodeWidth = NODE_WIDTH;

    if (gn->getName().length() > 18)
    {
        thisNodeWidth += (gn->getName().length() - 18) * 10;
    }
    
    gn->setLevel(level);
    gn->setHorzShift(offset);
    gn->setWidth(thisNodeWidth);
    gn->updateBoundaries();

}

void GraphViewer::removeNode(GenericProcessor *p)
{
    auto node = getNodeForEditor(p->getEditor());
    node->stillNeeded = false;
}

void GraphViewer::removeAllNodes()
{
    availableNodes.clear();
    repaint();
}


int GraphViewer::getIndexOfEditor (GenericEditor* editor) const
{
    int index = -1;
    
    const int numAvailableNodes = availableNodes.size();
    
    for (int i = 0; i < numAvailableNodes; ++i)
    {
        if (availableNodes[i]->hasEditor (editor))
        {
            return i;
        }
    }
    
    return index;
}


GraphNode* GraphViewer::getNodeForEditor (GenericEditor* editor) const
{
    int indexOfEditor = getIndexOfEditor (editor);
    
    if (indexOfEditor > -1)
        return availableNodes[indexOfEditor];
    else
        return nullptr;
}

int GraphViewer::getLevelStartY(int level) const
{
    if (levelStartY.count(level) > 0)
        return levelStartY.at(level);
    else
        return 0;
}

void GraphViewer::paint (Graphics& g)
{
    // Draw connections
    const int numAvailableNodes = availableNodes.size();

    for (int i = 0; i < numAvailableNodes; ++i)
    {
        if(availableNodes[i]->getSource() == nullptr)
        {   
            Path linePath;
            Point<float> startPoint = availableNodes[i]->getSrcPoint().translated(-15, 0);
            Point<float> endPoint = availableNodes[i]->getSrcPoint();

            linePath.startNewSubPath(startPoint);
            linePath.lineTo(endPoint);

            g.setColour(Colour(30, 30, 30));
            PathStrokeType stroke1(10.0f);
            g.strokePath(linePath, stroke1);

            g.setColour(Colour(90, 90, 90));
            PathStrokeType stroke2(7.5f);
            g.strokePath(linePath, stroke2);

            g.setColour(Colour(150, 150, 150));
            PathStrokeType stroke3(4.5f);
            g.strokePath(linePath, stroke3);

            Colour ellipseColour = Colour(30,30,30);
            ColourGradient ellipseGradient = ColourGradient(Colours::lightgrey,
                                                startPoint.x - 10.0f, startPoint.y,
                                                Colours::grey,
                                                startPoint.x, startPoint.y,
                                                true);

            g.setColour(ellipseColour);
            g.drawEllipse(startPoint.x - 20.f, startPoint.y - 10.0f, 20.f, 20.f, 2.f);
            g.setGradientFill(ellipseGradient);
            g.fillEllipse (startPoint.x - 19.f, startPoint.y - 9.f, 18.f, 18.f);

            int level = availableNodes[i]->getLevel();
            static const String letters = "ABCDEFGHI";            

            g.setColour(Colours::black);
            g.setFont(Font("Silkscreen", "Regular", 14));
            g.drawText (String::charToString(letters[level]), startPoint.x - 20, startPoint.y - 10, 20, 20, Justification::centred, true);

        }

        if (! availableNodes[i]->isSplitter())
        {
            if (availableNodes[i]->getDest() != nullptr)
            {
                int indexOfDest = getIndexOfEditor (availableNodes[i]->getDest());
                
                if (indexOfDest > -1)
                    connectNodes (i, indexOfDest, g);
            }
        }
        else
        {
            Array<GenericEditor*> editors = availableNodes[i]->getConnectedEditors();
            
            for (int path = 0; path < 2; ++path)
            {
                int indexOfDest = getIndexOfEditor (editors[path]);
                
                if (indexOfDest > -1)
                    connectNodes (i, indexOfDest, g);
            }
        }
    }
}


void GraphViewer::connectNodes (int node1, int node2, Graphics& g)
{
    
    juce::Point<float> start  = availableNodes[node1]->getDestPoint();
    juce::Point<float> end    = availableNodes[node2]->getSrcPoint();
    
    Path linePath;
    float x1 = start.getX();
    float y1 = start.getY();
    float x2 = end.getX();
    float y2 = end.getY();
    
    linePath.startNewSubPath (x1, y1);
    
    linePath.cubicTo (x2, y1,
                      x1, y2,
                      x2, y2);
    
    
    g.setColour (Colour(30,30,30));
    PathStrokeType stroke3 (3.5f);
    Path arrowPath;
    stroke3.createStrokedPath(arrowPath, linePath);
    g.fillPath(arrowPath);
    
    g.setColour (Colours::grey);
    PathStrokeType stroke2 (2.0f);
    Path arrowPath2;
    stroke2.createStrokedPath(arrowPath2, linePath);
    g.fillPath(arrowPath2);

    g.drawArrow(Line<float>(x2 - 9.f, y2, x2 + 1.0f, y2), 0.0f, 10.f, 10.f);
}

/// ------------------------------------------------------


DataStreamInfo::DataStreamInfo(DataStream* stream_, GenericEditor* editor, GraphNode* node_)
    :  stream(stream_)
    ,  node(node_)
{
    
    streamParameterEditorComponent = std::make_unique<Component>(stream->getName());

    auto pEditors = stream->createDefaultEditor();

    int yPos = pEditors.size() > 0 ? 5 : 0;
    const int rowWidthPixels = 170;
    const int rowHeightPixels = 20;

    for (auto paramEditor : pEditors)
    {
        // set parameter editor bounds
        paramEditor->setBounds(5, yPos, rowWidthPixels, rowHeightPixels);
        yPos += rowHeightPixels + 5;

        //transfer ownership to DataStreamInfo
        parameterEditors.add(paramEditor);

        streamParameterEditorComponent->addAndMakeVisible(parameterEditors.getLast());
    }
    
    editorHeight = yPos;
    heightInPixels = 60;

    if(parameterEditors.size() > 0)
    {
        parameterPanel = std::make_unique<ConcertinaPanel>();
        parameterPanel->addPanel(-1, streamParameterEditorComponent.get(), false);
        parameterPanel->setMaximumPanelSize(streamParameterEditorComponent.get(),
                                            editorHeight);

        parameterButton = new DataStreamButton(this, editor, "Parameters");
        parameterButton->addListener(this);
        parameterPanel->setCustomPanelHeader(streamParameterEditorComponent.get(), parameterButton, true);
        // button->removeMouseListener(button->getParentComponent());

        addAndMakeVisible(parameterPanel.get());
        parameterPanel->addMouseListener(this, true);
        parameterPanel->setBounds(0, 60, NODE_WIDTH, 20);

        heightInPixels += 20;
    }
    
}

DataStreamInfo::~DataStreamInfo()
{

}

void DataStreamInfo::paint(Graphics& g)
{
    g.setFont(Font("Fira Sans", "SemiBold", 14));
    g.setColour(Colour(30, 30, 30));
    g.drawRect(0, 0, getWidth(), getHeight(), 1);
    g.setColour(Colours::white.darker());
    g.fillRect(1, 0, getWidth() - 2, getHeight() - 1);
    g.fillRect(1, 0, 24, getHeight() - 1);

    int numEventChannels = stream->getEventChannels().size();
    int numSpikeChannels = stream->getSpikeChannels().size();

    String ttlText = numEventChannels == 1 ? "TTL Channel" : "TTL Channels";
    String spikeText = numSpikeChannels == 1 ? "Spike Channel" : "Spike Channels";

    g.setColour(Colours::black);
    g.drawText("@ " + String(stream->getSampleRate()) + " Hz", 30, 0, getWidth() - 30, 20, Justification::left);
    g.drawText(ttlText, 30, 20, getWidth() - 30, 20, Justification::left);
    g.drawText(spikeText, 30, 40, getWidth() - 30, 20, Justification::left);

    g.drawText(String(stream->getChannelCount()), 0, 0, 25, 20, Justification::centred);
    g.drawText(String(numEventChannels), 0, 20, 25, 20, Justification::centred);
    g.drawText(String(numSpikeChannels), 0, 40, 25, 20, Justification::centred);

}

void DataStreamInfo::buttonClicked(Button* button)
{
    DataStreamButton* dsb = (DataStreamButton*)button;

    if (dsb != parameterButton)
        return;

    // expand/collapse panel and inform node about new size
    if (button->getToggleState())
    {
        heightInPixels = editorHeight + 80;
        node->updateBoundaries();
        parameterPanel->setSize(NODE_WIDTH, editorHeight + 20);
        parameterPanel->expandPanelFully(streamParameterEditorComponent.get(), false);
    }
    else
    {
        heightInPixels = 80;
        node->updateBoundaries();
        parameterPanel->setSize(NODE_WIDTH, 20);
        parameterPanel->setPanelSize(streamParameterEditorComponent.get(), 0, false);
    }

    node->setDataStreamPanelSize(this, heightInPixels);
    node->updateGraphView();
}

int DataStreamInfo::getDesiredHeight() const
{
    return heightInPixels;
}

int DataStreamInfo::getMaxHeight() const
{
    if (parameterEditors.size() > 0)
        return editorHeight + 80;
    else
        return 60;
}


ProcessorParameterComponent::ProcessorParameterComponent(GenericProcessor* p)
    :  processor(p)
{
    
    editorComponent = std::make_unique<Component>(processor->getName() + " Parameters");

    auto editors = processor->createDefaultEditor();

    int yPos = editors.size() > 0 ? 5 : 0;
    const int rowWidthPixels = 170;
    const int rowHeightPixels = 20;

    for (auto editor : editors)
    {
        // set parameter editor bounds
        editor->setBounds(5, yPos, rowWidthPixels, rowHeightPixels);
        yPos += rowHeightPixels + 5;

        //transfer ownership to DataStreamInfo
        parameterEditors.add(editor);

        editorComponent->addAndMakeVisible(parameterEditors.getLast());
    }

    editorComponent->setBounds(0, 0, rowWidthPixels, yPos);
    addAndMakeVisible(editorComponent.get());

    heightInPixels = editorComponent->getHeight();
    
}

ProcessorParameterComponent::~ProcessorParameterComponent()
{

}

void ProcessorParameterComponent::paint(Graphics& g)
{
    g.setColour(Colour(30, 30, 30));
    g.drawRect(0, 0, getWidth(), getHeight(), 1);
    g.setColour(Colours::white.darker());
    g.fillRect(1, 0, getWidth() - 2, getHeight() - 1);
}

void ProcessorParameterComponent::updateView()
{
    for (auto editor : parameterEditors)
    {
        editor->updateView();
    }
}


DataStreamButton::DataStreamButton(DataStreamInfo* info_, GenericEditor* editor_, const String& text)
    : Button(text)
    , editor(editor_)
    , info(info_)
{
    setClickingTogglesState(true);
    setToggleState(false, dontSendNotification);

    pathOpen.addTriangle(7.0f, 5.0f, 10.5f, 12.0f, 14.0f, 5.0f);
    pathOpen.applyTransform(AffineTransform::scale(1.2f));

    pathClosed.addTriangle(8.0f, 4.0f, 8.0f, 11.0f, 15.0f, 7.5f);
    pathClosed.applyTransform(AffineTransform::scale(1.2f));
    
}


DataStreamButton::~DataStreamButton()
{

}

int DataStreamButton::getDesiredHeight() const
{
    return info->getDesiredHeight();
}

void DataStreamButton::paintButton(Graphics& g, bool isHighlighted, bool isDown)
{

    g.setColour(Colour(30, 30, 30));
    g.fillAll();

    g.setColour(Colours::lightgrey);
    g.fillRect(1, 0, 24, getHeight() - 1);

    if(getButtonText().equalsIgnoreCase("Parameters"))
        g.setColour(editor->getBackgroundColor().withSaturation(0.5f).withAlpha(0.7f));
    else
        g.setColour(editor->getBackgroundColor().withAlpha(0.5f));

    g.fillRect(25, 0, getWidth() - 26, getHeight() - 1);

    g.setColour(Colour(30, 30, 30));

    if (getToggleState())
        g.fillPath(pathOpen);
    else
        g.fillPath(pathClosed);

    g.setColour(Colours::white);
    g.drawText(getButtonText(), 30, 0, getWidth()-30, 20, Justification::left);

}

/// ------------------------------------------------------

GraphNode::GraphNode (GenericEditor* ed, GraphViewer* g)
: editor        (ed)
, processor     (ed->getProcessor())
, gv            (g)
, isMouseOver   (false)
, stillNeeded   (true)
, nodeWidth     (NODE_WIDTH)
{
    nodeId = processor->getNodeId();
    horzShift = 0;
    vertShift = 0;
    processorInfoVisible = false;
    
    infoPanel = std::make_unique<ConcertinaPanel> ();
    
    if (!processor->isMerger() && !processor->isSplitter())
    {
        // Add processor info panel
        processorParamComponent = std::make_unique<ProcessorParameterComponent>(processor);

        infoPanel->addPanel(-1, processorParamComponent.get(), false);
        infoPanel->setMaximumPanelSize(processorParamComponent.get(), 
                                       processorParamComponent->heightInPixels);

        processorParamHeader = std::make_unique<Component>(processor->getName() + " Header");
        processorParamHeader->setBounds(0, 0, processorParamComponent->getWidth(), 20);
        infoPanel->setCustomPanelHeader(processorParamComponent.get(), processorParamHeader.get(), false);
        processorParamHeader->removeMouseListener(processorParamHeader->getParentComponent());

        // Add data stream info panel and buttons
        for (auto stream : processor->getDataStreams())
        {

            DataStreamInfo* info = new DataStreamInfo(processor->getDataStream(stream->getKey()), editor, this);
            infoPanel->addPanel(-1, info, true);
            infoPanel->setMaximumPanelSize(info, info->getMaxHeight());
            dataStreamInfos.add(info);

            DataStreamButton* button = new DataStreamButton(info, editor, stream->getName());
            button->addListener(this);
            infoPanel->setCustomPanelHeader(info, button, true);
            button->removeMouseListener(button->getParentComponent());
            dataStreamButtons.add(button);
        }

        addAndMakeVisible(infoPanel.get());
        infoPanel->addMouseListener(this, true);
    }
   
    setBounds(X_BORDER_SIZE + getHorzShift() * NODE_WIDTH,
        Y_BORDER_SIZE + getLevel() * NODE_HEIGHT,
        nodeWidth,
        40);

    previousHeight = 0;
    verticalOffset = 0;

    nodeDropShadower.setOwner(this);
}


GraphNode::~GraphNode()
{
}


int GraphNode::getLevel() const
{
    return vertShift;
}


void GraphNode::setLevel (int level)
{
    vertShift = level;
    
}


int GraphNode::getHorzShift() const
{
    return horzShift;
}


void GraphNode::setHorzShift (int shift)
{
    horzShift = shift;
    
}

void GraphNode::setWidth(int width)
{
    nodeWidth = width;
}

void GraphNode::mouseEnter (const MouseEvent& m)
{
    isMouseOver = true;
    
    repaint();
}


void GraphNode::mouseExit (const MouseEvent& m)
{
    isMouseOver = false;
    
    repaint();
}


void GraphNode::mouseDown (const MouseEvent& m)
{    
    AccessClass::getEditorViewport()->highlightEditor(editor);

    if (processor->isMerger() || processor->isSplitter())
        return;

    if (m.getEventRelativeTo(this).y < 20)
    {
        if (processorInfoVisible)
        {    
            processorInfoVisible = false;
            updateBoundaries();
            infoPanel->setPanelSize(processorParamComponent.get(), 0, false);
        }
        else
        {
            processorInfoVisible = true;
            updateBoundaries();
            infoPanel->expandPanelFully(processorParamComponent.get(), false);
        }

        updateGraphView();
    }
}

void GraphNode::buttonClicked(Button* button)
{

    updateBoundaries();

    DataStreamButton* dsb = (DataStreamButton*)button;

    if (button->getToggleState())
        infoPanel->setPanelSize(dsb->getComponent(), dsb->getDesiredHeight() , false);
    else
        infoPanel->setPanelSize(dsb->getComponent(), 0, false);

    updateGraphView();
}


bool GraphNode::hasEditor (GenericEditor* ed) const
{
    if (ed == editor)
        return true;
    else
        return false;
}


bool GraphNode::isSplitter() const
{
    return editor->isSplitter();
}


bool GraphNode::isMerger() const
{
    return editor->isMerger();
}


GenericEditor* GraphNode::getDest() const
{
    return editor->getDestEditor();
}


GenericEditor* GraphNode::getSource() const
{
    GenericProcessor* sourceNode = editor->getProcessor()->getSourceNode();
    
    if (sourceNode != nullptr)
        return sourceNode->getEditor();
    else
        return nullptr;
}


Array<GenericEditor*> GraphNode::getConnectedEditors() const
{
    return editor->getConnectedEditors();
}


const String GraphNode::getName() const
{
    return editor->getDisplayName();
}


juce::Point<float> GraphNode::getCenterPoint() const
{
    juce::Point<float> center = juce::Point<float> (getX() + 11, getY() + 10);
    
    return center;
}

juce::Point<float> GraphNode::getSrcPoint() const
{
    juce::Point<float> center = juce::Point<float> (getX(), getY() + 10);
    
    return center;
}

juce::Point<float> GraphNode::getDestPoint() const
{
    juce::Point<float> center = juce::Point<float> (getRight(), getY() + 10);
    
    return center;
}


void GraphNode::updateBoundaries()
{

    int panelHeight = 20;

    if (!processor->isMerger() && !processor->isSplitter() && processorInfoVisible)
        panelHeight = processorParamComponent->heightInPixels + 20;

    for (auto dsb : dataStreamButtons)
    {
        if (dsb->getToggleState())
            panelHeight += dsb->getDesiredHeight() + 20;
        else
            panelHeight += 20;
    }

    infoPanel->setBounds(0, 0, NODE_WIDTH, panelHeight);

    int nodeY = gv->getLevelStartY(getLevel());
    if (nodeY == 0)
        setBounds(X_BORDER_SIZE + getHorzShift() * (NODE_WIDTH + X_BORDER_SIZE),
            Y_BORDER_SIZE + getLevel() * (NODE_HEIGHT + 35),
            nodeWidth,
            panelHeight);
    else
        setBounds(X_BORDER_SIZE + getHorzShift() * (NODE_WIDTH + X_BORDER_SIZE),
            nodeY,
            nodeWidth,
            panelHeight);
}


void GraphNode::updateGraphView()
{
    gv->updateBoundaries();
    gv->repaint();
}


void GraphNode::setDataStreamPanelSize(Component* panelComponent, int height)
{
    infoPanel->setPanelSize(panelComponent, height, false);
}


void GraphNode::updateStreamInfo()
{
    if (!processor->isMerger() && !processor->isSplitter())
    {
        LOGDD("Removing data stream info and buttons for node: ", processor->getName());

        infoPanel.reset(new ConcertinaPanel());

        dataStreamInfos.clear();
        dataStreamButtons.clear();

        processorParamComponent->updateView();
        processorInfoVisible = false;

        infoPanel->addPanel(-1, processorParamComponent.get(), false);
        infoPanel->setMaximumPanelSize(processorParamComponent.get(), 
                                       processorParamComponent->heightInPixels);

        processorParamHeader.reset(new Component(processor->getName() + " Header"));
        processorParamHeader->setBounds(0, 0, processorParamComponent->getWidth(), 20);
        infoPanel->setCustomPanelHeader(processorParamComponent.get(), processorParamHeader.get(), false);
        processorParamHeader->removeMouseListener(processorParamHeader->getParentComponent());

        for (auto stream : processor->getDataStreams())
        {
            LOGDD("Adding data stream info and buttons for stream: ", stream->getName());
            DataStreamInfo* info = new DataStreamInfo(processor->getDataStream(stream->getStreamId()), editor, this);
            infoPanel->addPanel(-1, info, true);
            infoPanel->setMaximumPanelSize(info, info->getMaxHeight());
            dataStreamInfos.add(info);

            DataStreamButton* button = new DataStreamButton(info, editor, stream->getName());
            button->addListener(this);
            infoPanel->setCustomPanelHeader(info, button, true);
            button->removeMouseListener(button->getParentComponent());
            dataStreamButtons.add(button);
        }

        addAndMakeVisible(infoPanel.get());
        infoPanel->addMouseListener(this, true);
    }
}


void GraphNode::verticalShift(int pixels)
{
 
    setBounds(getX(), getY() + pixels, getWidth(), getHeight());

    if (!processor->isSplitter())
    {
        if (processor->destNode != nullptr)
        {
            GraphNode* node = gv->getNodeForEditor(processor->destNode->getEditor());
            
            if (node != nullptr)
                node->verticalShift(pixels);
        }
            
    }
    else {
        SplitterEditor* editor = (SplitterEditor*)processor->getEditor();

        for (auto ed : editor->getConnectedEditors())
        {
            GraphNode* node = gv->getNodeForEditor(ed);
            
            if (node != nullptr)
                node->verticalShift(pixels);
        }

    }

    verticalOffset = pixels;

}

String GraphNode::getInfoString()
{
    GenericProcessor* processor = (GenericProcessor*) editor->getProcessor();
    
    int ch1 = processor->getTotalContinuousChannels();
    int ch2 = processor->getTotalEventChannels();
    int ch3 = processor->getTotalSpikeChannels();
    
    String info = "Data channels: ";
    info += String(ch1);
    
    info += "\nEvent channels: ";
    info += String(ch2);
    
    info += "\nSpike channels: ";
    info += String(ch3);
    
    return info;
}


void GraphNode::paint (Graphics& g)
{   
    g.setColour(Colour(30, 30, 30));
    g.fillRect(0, 0, getWidth(), 20);

    g.setColour(Colours::lightgrey);
    g.fillRect(1, 1, 24, 18);
    g.setColour(editor->getBackgroundColor());
    g.fillRect(25, 1, getWidth() - 26, 18);

    g.setFont(Font("Fira Code", "SemiBold", 14));
    g.setColour (Colours::black); // : editor->getBackgroundColor());
    g.drawText (String(nodeId), 1, 0, 23, 20, Justification::centred, true);
    g.setColour(Colours::white); // : editor->getBackgroundColor());
    g.drawText(getName(), 29, 0, getWidth() - 29, 20, Justification::left, true);
}

void GraphNode::paintOverChildren(Graphics& g)
{
    g.setColour(Colours::yellow);
    if (isMouseOver)
    {
        g.drawRect(0, 0, getWidth(), getHeight(), 2);
    }
}
