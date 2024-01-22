/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../../SimpleMultiBandComp/Source/GUI/RotarySliderWithLabels.h"
#include "../../SimpleMultiBandComp/Source/GUI/Utilities.h"
#include "../../SimpleMultiBandComp/Source/GUI/CustomButtons.h"

static juce::String getNameFromDSPOption(VoxProcessorAudioProcessor::DSP_Option option)
{
    switch(option)
    {
        case VoxProcessorAudioProcessor::DSP_Option::Phase:
            return "PHASE";
        case VoxProcessorAudioProcessor::DSP_Option::Chorus:
            return "CHORUS";
        case VoxProcessorAudioProcessor::DSP_Option::OverDrive:
            return "OVERDRIVE";
        case VoxProcessorAudioProcessor::DSP_Option::LadderFilter:
            return "LADDERFILTER";
        case VoxProcessorAudioProcessor::DSP_Option::GeneralFilter:
            return "GEN FILTER";
        case VoxProcessorAudioProcessor::DSP_Option::END_OF_LIST:
            jassertfalse;
    }
    return "NO SELECTION";
}

//==============================================================================
ExtendedTabbedButtonBar::ExtendedTabbedButtonBar() :
juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop)
{
    auto img = juce::Image(juce::Image::PixelFormat::SingleChannel, 1, 1, true);
    
    auto gfx = juce::Graphics(img);
    gfx.fillAll(juce::Colours::transparentWhite);
    
    dragImage = juce::ScaledImage(img, 1.0);
}

bool ExtendedTabbedButtonBar::isInterestedInDragSource (const SourceDetails& dragSourceDetails)
{
 
    if(dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
        return true;
    
    return false;
}

void ExtendedTabbedButtonBar::itemDragExit(const SourceDetails &dragSourceDetails)
{
//    DBG( "ExtendedTabbedButtonBar::itemDragExit");
    juce::DragAndDropTarget::itemDragExit(dragSourceDetails);
}

void ExtendedTabbedButtonBar::itemDropped (const SourceDetails& dragSourceDetails)
{
//    DBG("Item dropped");
    //Find the dropped item, lock the position in.
    resized();
    
    //This is to modify tab order
    auto tabs = getTabs();
    VoxProcessorAudioProcessor::DSP_Order newOrder;
    
    jassert(tabs.size() == newOrder.size());
    for (size_t i = 0; i < tabs.size(); ++i)
    {
        auto tab = tabs[static_cast<int>(i)];
        if (auto* etbb = dynamic_cast<ExtendedTabBarButton*>(tab)) {
            newOrder[i] = etbb->getOption();
        }
    }
    
    listeners.call([newOrder](Listener& l)
    {
        l.tabbedOrderChanged(newOrder);
    });
    
}

void ExtendedTabbedButtonBar::itemDragEnter(const SourceDetails &dragSourceDetails)
{
//    DBG("ExtendedTabbedButtonBar::itemDragEnter");
    juce::DragAndDropTarget::itemDragEnter(dragSourceDetails);
}

void ExtendedTabbedButtonBar::itemDragMove(const SourceDetails& dragSourceDetails)
{
    
    if(auto tabBarBeingDragged = dynamic_cast<ExtendedTabBarButton*>( dragSourceDetails.sourceComponent.get()) )
    {
        //find tabBarBeingDragged in the tabs[] container.
        //tabs[] is private so you must use:
        //TabBarButton* getTabButton (int index) const
        //and
        //getNumTabs()
        //to first get a list of tabs to search through
        
        auto numTabs = getNumTabs();
        auto tabs = juce::Array<juce::TabBarButton*>();
        tabs.resize(numTabs);
        for( int i = 0; i < numTabs; ++i )
        {
            tabs.getReference(i) = getTabButton(i);
        }
        
        //now search
        auto idx = findDraggedItemIndex(dragSourceDetails);
        if( idx == -1 )
        {
            DBG("failed to find tab being dragged in list of tabs");
            jassertfalse;
            return;
        }
        
        //find the tab that tabBarBeingDragged is colliding with.
        //it might be on the right
        //it might be on the left
        //if it's on the right,
        //if tabBarBeingDragged's x is > nextTab.getX() + nextTab.getWidth() * 0.5, swap their position
        auto previousTabIndex = idx - 1;
        auto nextTabIndex = idx + 1;
        auto previousTab = getTabButton( previousTabIndex );
        auto nextTab = getTabButton( nextTabIndex );
        
        auto centerX = tabBarBeingDragged->getBounds().getCentreX();
        
        if( previousTab == nullptr && nextTab != nullptr )
        {
            //you're in the 0th position
            if( centerX > nextTab->getX() )
            {
                moveTab(idx, nextTabIndex);
            }
        }
        else if( previousTab != nullptr && nextTab == nullptr )
        {
            //you're in the last position
            if( centerX < previousTab->getX() )
            {
                moveTab(idx, previousTabIndex);
            }
        }
        else
        {
            //you're in the middle
            if( centerX > nextTab->getX() )
            {
                moveTab(idx, nextTabIndex);
            }
            else if( centerX < previousTab->getRight() )
            {
                moveTab(idx, previousTabIndex);
            }
        }
        tabBarBeingDragged->toFront(true);
    }
}

void ExtendedTabbedButtonBar::mouseDown(const juce::MouseEvent& e)
{
//    DBG("ExtendedTabbedButtonBar::mouseDown");
    if(auto tabBarBeingDragged = dynamic_cast<ExtendedTabBarButton*>(e.originalComponent))
    {
        auto tabs = getTabs();
        auto idx = tabs.indexOf(tabBarBeingDragged);
        if (idx != -1) {
            setCurrentTabIndex(idx);
            setTabColours();
        }
        startDragging(tabBarBeingDragged->TabBarButton::getTitle(),tabBarBeingDragged, dragImage);
    }
}




juce::TabBarButton* ExtendedTabbedButtonBar::findDraggedItem(const SourceDetails &dragSourceDetails)
{
    return getTabButton(findDraggedItemIndex(dragSourceDetails));
}

int ExtendedTabbedButtonBar::findDraggedItemIndex(const SourceDetails &dragSourceDetails)
{
    if(auto tabBarBeingDragged = dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
    {
        auto tabs = getTabs();
        
        auto idx = tabs.indexOf(tabBarBeingDragged);
        return  idx;
    }
    
    return -1;
}

juce::Array<juce::TabBarButton*> ExtendedTabbedButtonBar::getTabs()
{
    auto numTabs = getNumTabs();
    auto tabs = juce::Array<juce::TabBarButton*>();
    tabs.resize(numTabs);
    for (int i = 0; i < numTabs; ++i) {
        tabs.getReference(i) = getTabButton(i);
    }
    
    auto unsorted = tabs;
    Comparator comparator;
    tabs.sort(comparator);
    
    return tabs;
}

HorizontalConstrainer::HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinerBoundsGetter,
                                             std::function<juce::Rectangle<int>()> confineeBoundsGetter) :
                                                                                                        boundsToConfineToGetter(confinerBoundsGetter),
                                                                                                        boundsOfConfineeGetter(confineeBoundsGetter)
{
    
}

void HorizontalConstrainer::checkBounds(juce::Rectangle<int>& bounds,
                                        const juce::Rectangle<int>& previousBounds,
                                        const juce::Rectangle<int>& limits,
                                        bool isStretchingTop,
                                        bool isStretchingLeft,
                                        bool isStretchingBottom,
                                        bool isStretchingRight)
{
    bounds.setY(previousBounds.getY());
    if(boundsToConfineToGetter != nullptr && boundsOfConfineeGetter != nullptr)
    {
        auto boundsToConfineTo = boundsToConfineToGetter();
        auto boundsOfConfinee = boundsOfConfineeGetter();
        
        bounds.setX(juce::jlimit(boundsToConfineTo.getX(),
                                 boundsToConfineTo.getRight() - boundsOfConfinee.getWidth(),
                                 bounds.getX()));
    }else{
        bounds.setX(juce::jlimit(limits.getX(),
                                 limits.getY(),
                                 bounds.getX()));
    }
}

ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name,
                                           juce::TabbedButtonBar& owner,
                                           VoxProcessorAudioProcessor::DSP_Option o) :
                                                                                        juce::TabBarButton (name, owner),
                                                                                        option(o)

{
    constrainer = std::make_unique<HorizontalConstrainer>([&owner]()
    {
        return owner.getLocalBounds();
    },
                                                          [this]()
                                                          {
        return getBounds();
    });
    constrainer->setMinimumOnscreenAmounts(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
}

int ExtendedTabBarButton::getBestTabLength(int depth)
{
    auto bestWidth = getLookAndFeel().getTabButtonBestWidth(*this, depth);
    
    auto& bar = getTabbedButtonBar();
    
    return juce::jmax(bestWidth, bar.getWidth()/bar.getNumTabs());
}

static VoxProcessorAudioProcessor::DSP_Option getDSPOptionFromName(juce::String name)
{
    if (name == "PHASE")
        return VoxProcessorAudioProcessor::DSP_Option::Phase;
    if (name == "CHORUS")
        return VoxProcessorAudioProcessor::DSP_Option::Chorus;
    if (name == "OVERDRIVE")
        return VoxProcessorAudioProcessor::DSP_Option::OverDrive;
    if (name == "LADDERFILTER")
        return VoxProcessorAudioProcessor::DSP_Option::LadderFilter;
    if (name == "GEN FILTER")
        return VoxProcessorAudioProcessor::DSP_Option::GeneralFilter;
    
    return VoxProcessorAudioProcessor::DSP_Option::END_OF_LIST;
}

void ExtendedTabbedButtonBar::setTabColours()
{
    auto tabs = getTabs();
    for (int i = 0; i < tabs.size(); ++i)
    {
        auto color = tabs[i]->isFrontTab() ? juce::Colours::darkgreen : juce::Colours::greenyellow;
        setTabBackgroundColour(i, color);
        tabs[i]->repaint();
    }
}

//=============================== CUSTOM POWER BUTTON ===============================================
struct PowerButtonWithParam : PowerButton
{
    PowerButtonWithParam(juce::RangedAudioParameter* p);
    void changedAttachment(juce::RangedAudioParameter* p);
private:
    std::unique_ptr<juce::ButtonParameterAttachment> attachment;
};

PowerButtonWithParam::PowerButtonWithParam(juce::RangedAudioParameter* p)
{
    jassert(p != nullptr);
    changedAttachment(p);
}

void PowerButtonWithParam::changedAttachment(juce::RangedAudioParameter *rap)
{
    attachment.reset();
    
    if (rap != nullptr) {
        
        attachment = std::make_unique<juce::ButtonParameterAttachment>(*rap, *this);
        attachment->sendInitialUpdate();
    
    }
}

//=========================== END OF CUSTOM POWER BUTTON ============================================

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton (const juce::String& tabName, int tabIndex)
{
    auto dspOption = getDSPOptionFromName(tabName);
    auto etbb = std::make_unique<ExtendedTabBarButton>(tabName, *this, dspOption);
    etbb->addMouseListener(this, false);
    return etbb.release();
}

void ExtendedTabbedButtonBar::addListener(ExtendedTabbedButtonBar::Listener* l)
{
    listeners.add(l);
}

void ExtendedTabbedButtonBar::removeListener(ExtendedTabbedButtonBar::Listener* l)
{
    listeners.remove(l);
}

void ExtendedTabbedButtonBar::currentTabChanged(int newCurrentTabIndex, const juce::String &newCurrentTabName)
{
    juce::ignoreUnused(newCurrentTabName);
    
    listeners.call([newCurrentTabIndex](Listener& l)
    {
        l.selectedTabChanged(newCurrentTabIndex);
    });
}

//============== DSP_GUI =======================================================

DSP_Gui::DSP_Gui(VoxProcessorAudioProcessor& proc) : processor(proc)
{
    
}

void DSP_Gui::resized()
{
    //buttons along the top.
    //combo boxes along the left
    //sliders take up the rest
    
    auto bounds = getLocalBounds();
    if( ! buttons.empty() )
    {
        auto buttonArea = bounds.removeFromTop(30);
        
        auto w = buttonArea.getWidth() / buttons.size();
        for( auto& button : buttons )
        {
            button->setBounds( buttonArea.removeFromLeft(static_cast<int>(w)));
        }
    }
    
    if( ! comboBoxes.empty() )
    {
        auto comboArea = bounds.removeFromLeft(150);
        
        auto h = juce::jmin(comboArea.getHeight() / static_cast<int>(comboBoxes.size()), 30);
        for( auto& cb : comboBoxes )
        {
            cb->setBounds( comboArea.removeFromTop( static_cast<int>(h) ));
        }
    }
    
    if( ! sliders.empty() )
    {
        auto w = bounds.getWidth() / sliders.size();
        for( auto& s : sliders )
        {
            s->setBounds( bounds.removeFromLeft(static_cast<int>(w)) );
        }
    }
}

void DSP_Gui::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::black);
}

void DSP_Gui::rebuildInterface(std::vector<juce::RangedAudioParameter*> params)
{
    if (params == currentParams) {
        return;
    }
    
    currentParams = params;
    
    sliderAttachments.clear();
    comboBoxAttachments.clear();
    buttonAttachments.clear();
    
    sliders.clear();
    comboBoxes.clear();
    buttons.clear();
    
    for (size_t i = 0; i < params.size(); ++i) 
    {
        auto p = params[i];
//        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(p)) {
//            comboBoxes.push_back(std::make_unique<juce::ComboBox>());
//            auto& cb = *comboBoxes.back();
//            cb.addItemList(choice->choices, 1);
//            comboBoxAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts, p->getName(100), cb));
//        }
         if( auto* toggle = dynamic_cast<juce::AudioParameterBool*>(p) )
        {
//            buttons.push_back( std::make_unique<juce::ToggleButton>("Bypass") );
//            auto& btn = *buttons.back();
//            buttonAttachments.push_back( std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts, p->getName(100), btn));
            DBG( "DSP_Gui::rebuildInterface() skipping APVTS::ButtonAttachment for AudioParameterBool: " << p->getName(100));
        }
        else
        {
            sliders.push_back(std::make_unique<RotarySliderWithLabels>(p, p->label, p->getName(100)));
            auto& slider = *sliders.back();
            
            SimpleMBComp::addLabelPairs(slider.labels, *p, p->label);
            slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
            
            sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, p->getName(100), slider));
        }
    }
    
    for(auto& slider : sliders)
        addAndMakeVisible(slider.get());
    for(auto& cb : comboBoxes)
        addAndMakeVisible(cb.get());
    for(auto& btn : buttons)
        addAndMakeVisible(btn.get());
    
    resized();
}

//=============== END OF DSP_GUI =======================================================

//==============================================================================
VoxProcessorAudioProcessorEditor::VoxProcessorAudioProcessorEditor (VoxProcessorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(tabbedComponent);
    addAndMakeVisible(dspGUI);
    
    inGainControl = std::make_unique<RotarySliderWithLabels>(audioProcessor.inputGain, "dB", "IN");
    outGainControl = std::make_unique<RotarySliderWithLabels>(audioProcessor.outputGain, "dB", "OUT");
    
    addAndMakeVisible(inGainControl.get());
    addAndMakeVisible(outGainControl.get());
    
    SimpleMBComp::addLabelPairs(inGainControl->labels, *audioProcessor.inputGain, "dB");
    SimpleMBComp::addLabelPairs(outGainControl->labels, *audioProcessor.outputGain, "dB");
        
    inGainAttachment = std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.inputGain, *inGainControl);
    outGainAttachment =  std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.outputGain, *outGainControl);
    
    audioProcessor.guiNeedsLatestDspOrder.set(true);
    
    tabbedComponent.addListener(this);
    startTimer(30);
    setSize (768, 450 + ioControlSize);
    
    //[DONE]: add bypass button to Tabs
    //[DONE]: make selected tab more obvious
    //[DONE]: mouse-down on tab (during drag) should change DSP_Gui
    //[DONE]: replace vertical sliders with SimpleMBComp rotary Sliders
    //[DONE] replace Comboboxes with SimpleMBComp combobox
    //[DONE]: replace bypass buttons with SimpleMBComp bypass buttons.
    //[DONE]: restore tab order when window opens first time (after quit).
    //[DONE]: restore tabs when closing/opening window (no quit)
    //[DONE]: restore selected tab when closing/opening window (no quit).
    //TODO: GUI design for each DSP instance?
    //TODO: fix graphic issue when dragging tab over bypass button
}

VoxProcessorAudioProcessorEditor::~VoxProcessorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    tabbedComponent.removeListener(this);
}

//==============================================================================
void VoxProcessorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    auto fillMeter = [&](auto rect, auto& rmsSource)
    {
        g.setColour(juce::Colours::black);
        g.fillRect(rect);
        
        auto rms = rmsSource.get();
        if (rms > 1.0f)
        {
            g.setColour(juce::Colours::red);
            //This defines a point at 0 dBFS
            auto lowerLeft = juce::Point<float>(rect.getX(),
                                                juce::jmap<float>
                                                (juce::Decibels::gainToDecibels(1.f),
                                                NEGATIVE_INFINITY,
                                                MAX_DECIBELS,
                                                rect.getBottom(),
                                                rect.getY()));
            //This defines a point at rms dBFS
            auto upperRight = juce::Point<float>(rect.getRight(),
                                                 juce::jmap<float>(juce::Decibels::gainToDecibels(rms),
                                                NEGATIVE_INFINITY,
                                                MAX_DECIBELS,
                                                rect.getBottom(),
                                                rect.getY()));
            //This defines a rectangle using those points
            auto overTHRect = juce::Rectangle<float>(lowerLeft, upperRight);
            //This fills that rectangle in red
            g.fillRect(overTHRect);
        }
        
        //This clamps the rms level to 1.0, since the over-threshold if() draws everything over 0dBFS
        rms = juce::jmin<float>(rms, 1.0);
        g.setColour(juce::Colours::green);
        //This draws the rectangle that represents the signal level below 0dBFS
        g.fillRect(rect.withY(juce::jmap<float>(juce::Decibels::gainToDecibels(rms),
                                                        NEGATIVE_INFINITY,
                                                        MAX_DECIBELS,
                                                        rect.getBottom(),
                                                        rect.getY()))
                           .withBottom(rect.getBottom()));
    };
    
    auto drawTicks = [&](auto rect, auto leftMeterRightEdge, auto rightMeterLeftEdge)
    {
        for (int i = MAX_DECIBELS; i >= NEGATIVE_INFINITY; i-=12)
        {
            auto y = juce::jmap<int>(i, NEGATIVE_INFINITY, MAX_DECIBELS, rect.getBottom(), rect.getY());
            auto r = juce::Rectangle<int>(rect.getWidth(), fontHeight);
            r.setCentre(rect.getCentreX(), y);
            
            g.setColour(i == 0 ? juce::Colours::whitesmoke :
                        i > 0 ? juce::Colours::red :
                        juce::Colours::teal);
            g.drawFittedText(juce::String(i), r, juce::Justification::centred, 1);
            
            if (i != MAX_DECIBELS && i != NEGATIVE_INFINITY)
            {
                g.drawLine(rect.getX() + tickIndent, y, leftMeterRightEdge - tickIndent, y);
                g.drawLine(rightMeterLeftEdge + tickIndent, y, rect.getRight() - tickIndent, y);
            }
        }
    };
    
    auto drawMeter = [&fillMeter, &drawTicks](auto rect, auto& g, const auto& leftSource, const auto& rightSource, const auto& label)
    {
        g.setColour(juce::Colours::teal);
        g.drawRect(rect);
        rect.reduce(2,2);
        
        g.setColour(juce::Colours::whitesmoke);
        g.drawText(label, rect.removeFromBottom(fontHeight), juce::Justification::centred);
        rect.removeFromTop(fontHeight / 2);
        
        const auto meterArea = rect;
        const auto leftChan = rect.removeFromLeft(meterChanWidth);
        const auto rightChan = rect.removeFromRight(meterChanWidth);
        
        fillMeter(leftChan, leftSource);
        fillMeter(rightChan, rightSource);
        drawTicks(meterArea, leftChan.getRight(), rightChan.getX());
    };

    auto bounds = getLocalBounds();
    bounds.removeFromBottom(ioControlSize);
    
    auto preMeterArea = bounds.removeFromLeft(meterWidth);
    drawMeter(preMeterArea,
              g,
              audioProcessor.leftPreRMS,
              audioProcessor.rightPreRMS,
              "Input");
    auto postMeterArea = bounds.removeFromRight(meterWidth);
    drawMeter(postMeterArea,
              g,
              audioProcessor.leftPostRMS, 
              audioProcessor.rightPostRMS,
              "Output");
//
//    fillMeter(preMeterArea.removeFromLeft(preMeterArea.getWidth() / 2), audioProcessor.leftPreRMS);
//    fillMeter(preMeterArea, audioProcessor.rightPreRMS);
//    fillMeter(postMeterArea.removeFromLeft(postMeterArea.getWidth() / 2), audioProcessor.leftPostRMS);
//    fillMeter(postMeterArea, audioProcessor.rightPostRMS);
//    
//    drawTicks(leftMeterArea, leftMeterArea.removeFromLeft(meterChanWidth).getRight(), leftMeterArea.removeFromRight(meterChanWidth).getX());
//    drawTicks(rightMeterArea, rightMeterArea.removeFromLeft(meterChanWidth).getRight(), rightMeterArea.removeFromRight(meterChanWidth).getX());
    
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Morris Sound", getLocalBounds(), juce::Justification::centred, 1);
}

void VoxProcessorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    bounds.removeFromTop(10);
    
    auto gainArea = bounds.removeFromBottom(ioControlSize);
    inGainControl->setBounds(gainArea.removeFromLeft(ioControlSize));
    outGainControl->setBounds(gainArea.removeFromRight(ioControlSize));
    
    auto leftMeterArea = bounds.removeFromLeft(meterWidth);
    auto rightMeterArea = bounds.removeFromRight(meterWidth);
    
    juce::ignoreUnused(leftMeterArea, rightMeterArea);
    
    tabbedComponent.setBounds(bounds.removeFromTop(30));
    dspGUI.setBounds(bounds);
    
}

void VoxProcessorAudioProcessorEditor::timerCallback()
{
    repaint();
    if(audioProcessor.restoreDspOrderFifo.getNumAvailableForReading() == 0)
        return;
    
    using T = VoxProcessorAudioProcessor::DSP_Order;
    T newOrder;
    newOrder.fill(VoxProcessorAudioProcessor::DSP_Option::END_OF_LIST);
    auto empty = newOrder;
    while (audioProcessor.restoreDspOrderFifo.pull(newOrder))
    {
        ;
    }
    
    if(newOrder != empty)
    {
        addTabsFromDSPOrder(newOrder);
    }
    
    if (selectedTabAttachment == nullptr)
    {
        selectedTabAttachment = std::make_unique<juce::ParameterAttachment>(*audioProcessor.selectedTab,
            [this](float tabNum)
            {
                auto newTabNum = static_cast<int>(tabNum);
                if(juce::isPositiveAndBelow(newTabNum, tabbedComponent.getNumTabs()))
                {
                    tabbedComponent.setCurrentTabIndex(newTabNum);
                }
                else
                {
                    jassertfalse;
                }
            });
        
        selectedTabAttachment->sendInitialUpdate();
    }
}

void VoxProcessorAudioProcessorEditor::tabbedOrderChanged(VoxProcessorAudioProcessor::DSP_Order newOrder)
{
    rebuildInterface();
    audioProcessor.dspOrderFifo.push(newOrder);
}

void VoxProcessorAudioProcessorEditor::addTabsFromDSPOrder(VoxProcessorAudioProcessor::DSP_Order newOrder)
{
    tabbedComponent.clearTabs();
    for (auto v : newOrder) {
        tabbedComponent.addTab(getNameFromDSPOption(v), juce::Colours::greenyellow, -1);
    }
    
    /*
         Bypass buttons are added to the tabs AFTER they have been created and added to the tabbed component.
         The mechanism used is kind of ugly
         the DSP order is used to retrieve the params for a DSP_Option.
         then the params are searched. We are looking for an AudioParameterBool instance.
         then the found AudioParameterBool is checked to see if the param name contains "bypass".
         If those two conditions are met, the bypass param has been found.
         now the button can be created, configured, and added to the tab as an extra component.
         */
    
    auto numTabs = tabbedComponent.getNumTabs();
    auto size = tabbedComponent.getHeight();
    
    for (int i = 0; i < numTabs; ++i) 
    {
        if (auto tab = tabbedComponent.getTabButton(i)) 
        {
            auto order = newOrder[i];
            auto params = audioProcessor.getParamsForOption(order);
            for( auto p : params)
            {
                if (auto bypass = dynamic_cast<juce::AudioParameterBool*>(p))
                {
                    if( bypass->name.containsIgnoreCase("bypass") )
                    {
                        auto pbwp = std::make_unique<PowerButtonWithParam>(bypass);
                        pbwp->setSize(size, size);
                        tab->setExtraComponent(pbwp.release(), juce::TabBarButton::ExtraComponentPlacement::beforeText);
                    }
                }
            }
        }
    }
    tabbedComponent.setTabColours();
    rebuildInterface();
    audioProcessor.dspOrderFifo.push(newOrder);
}

void VoxProcessorAudioProcessorEditor::rebuildInterface()
{
    auto currentTabIndex = tabbedComponent.getCurrentTabIndex();
    std::cout << "Current Tab Index: " << currentTabIndex << std::endl;
    auto currentTab = tabbedComponent.getTabButton(currentTabIndex);
    if( auto etab = dynamic_cast<ExtendedTabBarButton*>(currentTab) )
    {
        auto option = etab->getOption();
        DBG("Option value: " << static_cast<int>(option));
        auto params = audioProcessor.getParamsForOption(option);
        jassert( ! params.empty() );
        dspGUI.rebuildInterface(params);
    }
}

void VoxProcessorAudioProcessorEditor::selectedTabChanged(int newCurrentTabIndex)
{
    /*
         Selected Tab restoration requires a ParamAttachment to exist.
         the attachment is also in charge of setting the Selected Tab parameter when the user clicks on a tab.
         when the tab is changed, the interface is also rebuilt.
         when the audio parameter settings are loaded from disk, the callback for the parameter attachment is called.
         this callback changes the selected tab and rebuilds the interface.
         the creation of the attachment can't happen until after tabs have been created.
         tabs are created in TimerCallback whenever the restoreDspOrderFifo has a DSP_Order instance to pull.
         This is why the attachment creation is not in the constructor, but is instead in timerCallback(), after it is determined that the restoreDspOrderFifo has items to pull.
         */
    if (selectedTabAttachment)
    {
        rebuildInterface();
        selectedTabAttachment->setValueAsCompleteGesture(static_cast<float>(newCurrentTabIndex));
    }
}
