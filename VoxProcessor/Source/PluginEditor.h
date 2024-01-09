/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

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
    };
    
    void addListener(Listener* l);
    void removeListener(Listener* l);
    
private:
    juce::TabBarButton* findDraggedItem(const SourceDetails& dragSourceDetails);
    int findDraggedItemIndex(const SourceDetails& dragSourceDetails);
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

struct DSP_GUI : juce::Component
{
    DSP_GUI(VoxProcessorAudioProcessor& proc) : processor(proc) {}
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void rebuildInterface(std::vector<juce::RangedAudioParameter*> params);
    
    VoxProcessorAudioProcessor& processor;
    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::ComboBox>> comboBoxes;
    std::vector<std::unique_ptr<juce::Button>> buttons;
    
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboBoxAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
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

    void timerCallback() override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VoxProcessorAudioProcessor& audioProcessor;
    DSP_GUI dspGUI { audioProcessor };
    
//    juce::TabbedComponent tabbedComponent { juce::TabbedButtonBar::Orientation::TabsAtTop};
    ExtendedTabbedButtonBar tabbedComponent;
    void addTabsFromDSPOrder(VoxProcessorAudioProcessor::DSP_Order);
    void rebuildInterface();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxProcessorAudioProcessorEditor)
};
