/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

auto getPhaserRateName() { return juce::String("Phaser RateHz"); }
auto getPhaserCenterFreqName() { return juce::String("Phaser Center FreqHz"); }
auto getPhaserDepthName() { return juce::String("Phaser Depth %" ); }
auto getPhaserFeedbackName() { return juce::String("Phaser Feedback %" ); }
auto getPhaserMixName() { return juce::String("Phaser Mix %"); }

auto getChorusRateName() { return juce::String("Chorus RateHz"); }
auto getChorusDepthName() { return juce::String("Chorus Depth %"); }
auto getChorusCenterDelayName() { return juce::String("Chorus Center Delay ms" ); }
auto getChorusFeedbackName() { return juce::String("Chorus Feedback %" ); }
auto getChorusMixName() { return juce::String("Chorus Mix %"); }


//==============================================================================
VoxProcessorAudioProcessor::VoxProcessorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    auto floatParams = std::array
    {
        &phaserRateHz,
        &phaserCenterFreqHz,
        &phaserDepthPercent,
        &phaserFeedbackPercent,
        &phaserMixPercent,
        
        &chorusRateHz,
        &chorusDepthPercent,
        &chorusCenterDelayMs,
        &chorusFeedbackPercent,
        &chorusMixPercent,
    };
    
    auto floatNameFuncs = std::array
    {
        &getPhaserRateName,
        &getPhaserCenterFreqName,
        &getPhaserDepthName,
        &getPhaserFeedbackName,
        &getPhaserMixName,
        
        &getChorusRateName,
        &getChorusDepthName,
        &getChorusCenterDelayName,
        &getChorusFeedbackName,
        &getChorusMixName,
    };
    
    jassert( floatParams.size() == floatNameFuncs.size() );
    for (size_t i = 0; i < floatParams.size(); ++i)
    {
        auto ptrToParamPtr = floatParams[i];
        *ptrToParamPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(floatNameFuncs[i]()));
        jassert(*ptrToParamPtr != nullptr);
    }
}

VoxProcessorAudioProcessor::~VoxProcessorAudioProcessor()
{
}

//==============================================================================
const juce::String VoxProcessorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VoxProcessorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VoxProcessorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VoxProcessorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VoxProcessorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VoxProcessorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VoxProcessorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VoxProcessorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VoxProcessorAudioProcessor::getProgramName (int index)
{
    return {};
}

void VoxProcessorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VoxProcessorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void VoxProcessorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VoxProcessorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

juce::AudioProcessorValueTreeState::ParameterLayout VoxProcessorAudioProcessor::createParameterLayour()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    const int versionHint = 1;
    
    //phaser rate LFO Hz
    auto name = getPhaserRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                           name,
                                                           juce::NormalisableRange<float>(0.01f, 2.f, 0.01f, 1.f),
                                                           0.2f,
                                                           "Hz"));
    //phaser depth: 0 - 1
    name = getPhaserDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                           name,
                                                           juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
                                                            0.05f,
                                                            "%"));
    //phaser center freq: audio Hz
    name = getPhaserCenterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                            name,
                                                            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
                                                            1000.f,
                                                            "Hz"));
    //phaser feedback: -1 to 1
    name = getPhaserFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                            name,
                                                            juce::NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f),
                                                            0.0f,
                                                            "%"));
    //phaser mix: 0 - 1
    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                            name,
                                                            juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
                                                            0.05f,
                                                            "%"));
    
    //rate: Hz
    name = getChorusRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                            name,
                                                            juce::NormalisableRange<float>(0.01f, 100.f, 0.01f, 1.f),
                                                            0.2f,
                                                            "Hz"));
    //depth: 0 to 1
    name = getChorusDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                            name,
                                                            juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
                                                            0.05f,
                                                            "%"));
    //centre delay: milliseconds (1 to 100)
    name = getChorusCenterDelayName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                            name,
                                                            juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f),
                                                            7.f,
                                                            "%"));
    //feedback: -1 to 1
    name = getChorusFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                            name,
                                                            juce::NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f),
                                                            0.0f,
                                                            "%"));
    //mix: 0 to 1
    name = getChorusMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                            name,
                                                            juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
                                                            0.05f,
                                                            "%"));
    
    return layout;
}

void VoxProcessorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    //[DONE]: add APVTS
    //TODO: create audio parameters for all dsp choices
    //TODO: update DSP here from audio parameters
    //TODO: save/load settings
    //TODO: save/load DSP order
    //TODO: Drag-To-Reorder GUI
    //TODO: GUI design for each DSP instance?
    //TODO: metering
    //TODO: prepare all DSP
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    //Temp instance to pull into
    auto newDSPOrder = DSP_Order(); // <-- This is just an array
    
    //Try to pull from the Fifo
    while(dspOrderFifo.pull(newDSPOrder))
    {
        
    }
    //If pull succeeded, we refresh dspOrder
    if(newDSPOrder != DSP_Order())
        dspOrder = newDSPOrder;
    
    //convert dspOrder into an array of pointers to the DSP objects
    DSP_Pointers dspPointers;
    
    //We reasign the objects to their pointers if needed
    for(size_t i = 0; i < dspPointers.size(); ++i)
    {
        switch (dspOrder[i])
        {
            case DSP_Option::Phase:
                dspPointers[i] = &phaser;
                break;
            case DSP_Option::Chorus:
                dspPointers[i] = &chorus;
                break;
            case DSP_Option::OverDrive:
                dspPointers[i] = &overdrive;
                break;
            case DSP_Option::LadderFilter:
                dspPointers[i] = &ladderFilter;
                break;
            case DSP_Option::END_OF_LIST:
                jassertfalse;
                break;
        }
    }
    
    //Now, with the list up to date we can process
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    for(size_t i = 0; i < dspPointers.size(); ++i)
    {
        if(dspPointers[i] != nullptr)
        {
            dspPointers[i]->process(context);
        }
    }
}

//==============================================================================
bool VoxProcessorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VoxProcessorAudioProcessor::createEditor()
{
    return new VoxProcessorAudioProcessorEditor (*this);
}

//==============================================================================
void VoxProcessorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VoxProcessorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VoxProcessorAudioProcessor();
}
