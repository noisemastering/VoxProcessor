/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::String getDSPOptionName(VoxProcessorAudioProcessor::DSP_Option option)
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
    
}

bool ExtendedTabbedButtonBar::isInterestedInDragSource (const SourceDetails& dragSourceDetails)
{
 
    if(dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
        return true;
    
    return false;
}

void ExtendedTabbedButtonBar::itemDragExit(const SourceDetails &dragSourceDetails)
{
    DBG( "ExtendedTabbedButtonBar::itemDragExit");
    juce::DragAndDropTarget::itemDragExit(dragSourceDetails);
}

void ExtendedTabbedButtonBar::itemDropped (const SourceDetails& dragSourceDetails)
{
    DBG("Item dropped");
    //Find the dropped item, lock the position in.
}

void ExtendedTabbedButtonBar::itemDragEnter(const SourceDetails &dragSourceDetails)
{
    DBG("ExtendedTabbedButtonBar::itemDragEnter");
    juce::DragAndDropTarget::itemDragEnter(dragSourceDetails);
}

void ExtendedTabbedButtonBar::itemDragMove(const SourceDetails& dragSourceDetails)
{
    DBG( "ETBB::itemDragMove" );
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
        auto idx = tabs.indexOf(tabBarBeingDragged);
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
        
        if( previousTab == nullptr && nextTab != nullptr )
        {
            //you're in the 0th position
            if( tabBarBeingDragged->getX() > nextTab->getX() + nextTab->getWidth() * 0.5 )
            {
                moveTab(idx, nextTabIndex);
            }
        }
        else if( previousTab != nullptr && nextTab == nullptr )
        {
            //you're in the last position
            if( tabBarBeingDragged->getX() < previousTab->getX() + previousTab->getWidth() * 0.5 )
            {
                moveTab(idx, previousTabIndex);
            }
        }
        else
        {
            //you're in the middle
            if( tabBarBeingDragged->getX() > nextTab->getX() + nextTab->getWidth() * 0.5 )
            {
                moveTab(idx, nextTabIndex);
            }
            else if( tabBarBeingDragged->getX() < previousTab->getX() + previousTab->getWidth() * 0.5 )
            {
                moveTab(idx, previousTabIndex);
            }
        }
    }
}

void ExtendedTabbedButtonBar::mouseDown(const juce::MouseEvent& e)
{
    DBG("ExtendedTabbedButtonBar::mouseDown");
    if(auto tabBarBeingDragged = dynamic_cast<ExtendedTabBarButton*>(e.originalComponent))
    {
        startDragging(tabBarBeingDragged->TabBarButton::getTitle(),tabBarBeingDragged);
    }
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton (const juce::String& tabName, int tabIndex)
{
    auto etbb = std::make_unique<ExtendedTabBarButton>(tabName, *this);
    etbb->addMouseListener(this, false);
    return etbb.release();
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

ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner) : juce::TabBarButton (name, owner)
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

//==============================================================================
VoxProcessorAudioProcessorEditor::VoxProcessorAudioProcessorEditor (VoxProcessorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    dspOprderButton.onClick = [this]()
    {
        juce::Random r;
        VoxProcessorAudioProcessor::DSP_Order dspOrder;
        
        auto range = juce::Range<int>(static_cast<int>(VoxProcessorAudioProcessor::DSP_Option::Phase),
                                      static_cast<int>(VoxProcessorAudioProcessor::DSP_Option::END_OF_LIST));
        tabbedComponent.clearTabs();
        
        for( auto& v : dspOrder )
        {
            auto entry = r.nextInt(range);
            v = static_cast<VoxProcessorAudioProcessor::DSP_Option>(entry);
            tabbedComponent.addTab(getDSPOptionName(v), juce::Colours::orange, -1);
        }
        DBG(juce::Base64::toBase64(dspOrder.data(), dspOrder.size()));
//        jassertfalse;
        
        audioProcessor.dspOrderFifo.push(dspOrder);
    };
    
    addAndMakeVisible(dspOprderButton);
    addAndMakeVisible(tabbedComponent);
    
    setSize (400, 300);
}

VoxProcessorAudioProcessorEditor::~VoxProcessorAudioProcessorEditor()
{
}

//==============================================================================
void VoxProcessorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void VoxProcessorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    dspOprderButton.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(150, 30));
    bounds.removeFromTop(10);
    tabbedComponent.setBounds(bounds.withHeight(30));
}
