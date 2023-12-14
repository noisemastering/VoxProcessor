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

ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner) : juce::TabBarButton (name, owner)
{
    
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String &tabName, int tabIndex)
{
    return new ExtendedTabBarButton(tabName, *this);
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
