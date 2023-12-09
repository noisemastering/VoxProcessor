/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <Fifo.h>

//==============================================================================
/**
*/
class VoxProcessorAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    VoxProcessorAudioProcessor();
    ~VoxProcessorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    enum class DSP_Option
    {
        Phase,
        Chorus,
        OverDrive,
        LadderFilter,
        GeneralFilter,
        END_OF_LIST
    };
    
    using DSP_Order = std::array<DSP_Option, static_cast<size_t>(DSP_Option::END_OF_LIST)>;
    using DSP_Pointers = std::array<juce::dsp::ProcessorBase*, static_cast<size_t>(DSP_Option::END_OF_LIST)>;
    SimpleMBComp::Fifo<DSP_Order> dspOrderFifo;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayour();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Settings", createParameterLayour()};
    
    
    //Cached parameter instances
    juce::AudioParameterFloat* phaserRateHz = nullptr;
    juce::AudioParameterFloat* phaserCenterFreqHz = nullptr;
    juce::AudioParameterFloat* phaserDepthPercent = nullptr;
    juce::AudioParameterFloat* phaserFeedbackPercent = nullptr;
    juce::AudioParameterFloat* phaserMixPercent = nullptr;
    
    juce::AudioParameterFloat* chorusRateHz = nullptr;
    juce::AudioParameterFloat* chorusDepthPercent = nullptr;
    juce::AudioParameterFloat* chorusCenterDelayMs = nullptr;
    juce::AudioParameterFloat* chorusFeedbackPercent = nullptr;
    juce::AudioParameterFloat* chorusMixPercent = nullptr;
    
    juce::AudioParameterFloat* overdriveSaturation = nullptr;
    
    juce::AudioParameterChoice* ladderFilterMode = nullptr;
    juce::AudioParameterFloat* ladderFilterCutoffHz = nullptr;
    juce::AudioParameterFloat* ladderFilterResonance = nullptr;
    juce::AudioParameterFloat* ladderFilterDrive = nullptr;
    
    juce::AudioParameterChoice* generalFilterMode = nullptr;
    juce::AudioParameterFloat* generalFilterFreqHz = nullptr;
    juce::AudioParameterFloat* generalFilterQuality = nullptr;
    juce::AudioParameterFloat* generalFilterGain = nullptr;
    
    juce::AudioParameterBool* phaserBypass = nullptr;
    juce::AudioParameterBool* chorusBypass = nullptr;
    juce::AudioParameterBool* overdriveBypass = nullptr;
    juce::AudioParameterBool* ladderFilterBypass = nullptr;
    juce::AudioParameterBool* generalFilterBypass = nullptr;
    
private:
    //==============================================================================
    DSP_Order dspOrder;
    
    template<typename DSP>
    struct DSP_Choice : juce::dsp::ProcessorBase
    {
        void prepare(const juce::dsp::ProcessSpec &spec) override
        {
            dsp.prepare(spec);
        }
        void process(const juce::dsp::ProcessContextReplacing<float>& context) override
        {
            dsp.process(context);
        }
        void reset() override
        {
            dsp.reset();
        }
        
        DSP dsp;
    };
    
    DSP_Choice<juce::dsp::DelayLine<float>> delay;
    DSP_Choice<juce::dsp::Phaser<float>> phaser;
    DSP_Choice<juce::dsp::Chorus<float>> chorus;
    DSP_Choice<juce::dsp::LadderFilter<float>> overdrive, ladderFilter;
    DSP_Choice<juce::dsp::IIR::Filter<float>> generalFilter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxProcessorAudioProcessor)
};
