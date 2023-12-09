/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
        
        for( auto& v : dspOrder )
        {
            auto entry = r.nextInt(range);
            v = static_cast<VoxProcessorAudioProcessor::DSP_Option>(entry);
        }
        DBG(juce::Base64::toBase64(dspOrder.data(), dspOrder.size()));
        jassertfalse;
        
        audioProcessor.dspOrderFifo.push(dspOrder);
    };
    
    addAndMakeVisible(dspOprderButton);
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
    
    dspOprderButton.setBounds(getLocalBounds().reduced(100));
}
