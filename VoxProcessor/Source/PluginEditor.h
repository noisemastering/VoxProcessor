/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <LookAndFeel.h>

//==============================================================================
/**
*/

static constexpr int NEGATIVE_INFINITY = -72;
static constexpr int MAX_DECIBELS = 12;

struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget, juce::DragAndDropContainer
{
    ExtendedTabbedButtonBar();
    
    bool isInterestedInDragSource (const SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragMove (const SourceDetails& dragSourceDetails) override;
    void itemDragExit (const SourceDetails& dragSourceDetails) override;
    void itemDropped (const SourceDetails& dragSourceDetails) override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    
    juce::TabBarButton* createTabButton (const juce::String& tabName, int tabIndex) override;
    
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void tabbedOrderChanged(VoxProcessorAudioProcessor::DSP_Order newOrder) = 0;
        virtual void selectedTabChanged(int newCurrentTabIndex) = 0;
    };
    
    void addListener(Listener* l);
    void removeListener(Listener* l);
    void currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName) override;
    
private:
    juce::TabBarButton* findDraggedItem(const SourceDetails& dragSourceDetails);
    int findDraggedItemIndex(const SourceDetails& dragSourceDetails);
    struct Comparator
    {
        /*
        This will use a comparator object to sort the elements into order. The object
        passed must have a method of the form:
        @code
        int compareElements (ElementType first, ElementType second);
        @endcode

        ..and this method must return:
          - a value of < 0 if the first comes before the second
          - a value of 0 if the two objects are equivalent
          - a value of > 0 if the second comes before the first
         */
        int compareElements(juce::TabBarButton* first, juce::TabBarButton* second)
        {
            if( first->getX() < second->getX() )
                return -1;
            if( first->getX() == second->getX() )
                return 0;
            
            return 1;
        }
    };
    juce::Array<juce::TabBarButton*> getTabs();
    juce::ScaledImage dragImage;
    juce::ListenerList<Listener> listeners;
    
};

struct HorizontalConstrainer : juce::ComponentBoundsConstrainer
{
    HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinerBoundsGetter,
                          std::function<juce::Rectangle<int>()> confineeBoundsGetter);
    
    void checkBounds(juce::Rectangle<int>& bounds,
                     const juce::Rectangle<int>& previousBounds,
                     const juce::Rectangle<int>& limits,
                     bool isStretchingTop,
                     bool isStretchingLeft,
                     bool isStretchingBottom,
                     bool isStretchingRight) override;
    
private:
    std::function<juce::Rectangle<int>()> boundsToConfineToGetter;
    std::function<juce::Rectangle<int>()> boundsOfConfineeGetter;
};


struct ExtendedTabBarButton : juce::TabBarButton
{
    ExtendedTabBarButton(const juce::String& name, 
                         juce::TabbedButtonBar& owner,
                         VoxProcessorAudioProcessor::DSP_Option dspOption);
    
    juce::ComponentDragger dragger;
    std::unique_ptr<HorizontalConstrainer> constrainer;
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        toFront(true);
        dragger.startDraggingComponent(this, e);
        juce::TabBarButton::mouseDown(e);
    }
    
    void mouseDrag(const juce::MouseEvent& e) override
    {
        dragger.dragComponent(this, e, constrainer.get());
    }
    
    VoxProcessorAudioProcessor::DSP_Option getOption() const {return option;}
    
    int getBestTabLength(int depth) override;
    
private:
    VoxProcessorAudioProcessor::DSP_Option option;
};

struct RotarySliderWithLabels; //Forward declaration
struct DSP_Gui : juce::Component
{
    DSP_Gui(VoxProcessorAudioProcessor& proc);
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void rebuildInterface(std::vector<juce::RangedAudioParameter*> params);
    
    VoxProcessorAudioProcessor& processor;
    std::vector<std::unique_ptr<RotarySliderWithLabels>> sliders;
    std::vector<std::unique_ptr<juce::ComboBox>> comboBoxes;
    std::vector<std::unique_ptr<juce::Button>> buttons;
    
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboBoxAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
    
    std::vector<juce::RangedAudioParameter*> currentParams;
};

class VoxProcessorAudioProcessorEditor  : public juce::AudioProcessorEditor, 
                                                ExtendedTabbedButtonBar::Listener,
                                                juce::Timer
{
public:
    VoxProcessorAudioProcessorEditor (VoxProcessorAudioProcessor&);
    ~VoxProcessorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void tabbedOrderChanged(VoxProcessorAudioProcessor::DSP_Order) override;
    void selectedTabChanged(int newCurrentTabIndex) override;

    void timerCallback() override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VoxProcessorAudioProcessor& audioProcessor;
    LookAndFeel lookAndFeel;
    DSP_Gui dspGUI { audioProcessor };
    
    ExtendedTabbedButtonBar tabbedComponent;
    
    static constexpr int meterWidth = 80;
    static constexpr int fontHeight = 24;
    static constexpr int tickIndent = 8;
    static constexpr int meterChanWidth = 24;
    
    std::unique_ptr<juce::ParameterAttachment> selectedTabAttachment;
    
    void addTabsFromDSPOrder(VoxProcessorAudioProcessor::DSP_Order);
    void rebuildInterface();
    
    static constexpr int NEGATIVE_INFINITY = -72;
    static constexpr int MAX_DECIBELS = 12;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxProcessorAudioProcessorEditor)
};
