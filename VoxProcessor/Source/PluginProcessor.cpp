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

auto getOverdriveSaturationName() { return juce::String("Overdrive Saturation"); }
auto getLadderFilterModeName() { return juce::String("Ladder Filter Mode"); }
auto getLadderFilterCutoffName() { return juce::String("Ladder Filter Cuttoff"); }
auto getLadderFilterResonanceName() { return juce::String("Ladder Filter Resonance"); }
auto getLadderFilterDriveName() { return juce::String("Ladder Filter Drive"); }

auto getLadderFilterChoices()
{
    return juce::StringArray
    {
        "LPF12",
        "HPF12",
        "BPF12",
        "LPF24",
        "HPF24",
        "BPF24",
    };
}

auto getGenearlFilterChoices()
{
    return juce::StringArray
    {
        "Peak",
        "bandpass",
        "notch",
        "allpass",
    };
}

auto getGeneralFilterModeName() { return juce::String("General Filter Mode"); }
auto getGeneralFilterFreqName() { return juce::String("General Filter Freq Hz"); }
auto getGeneralFilterQualityName() { return juce::String("General Filter Quality"); }
auto getGeneralFilterGainName() { return juce::String("General Filter Gain"); }

auto getPhaserBypassName() { return juce::String("Phaser Bypass"); }
auto getChorusBypassName() { return juce::String("Chorus Bypass"); }
auto getOverdriveBypassName() { return juce::String("Overdrive Bypass"); }
auto getLadderFilterBypassName() { return juce::String("Ladder Filter Bypass"); }
auto getGeneralFilterBypassName() { return juce::String("General Filter Bypass"); }

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
//    dspOrder =
//    {{
//        DSP_Option::Phase,
//        DSP_Option::Chorus,
//        DSP_Option::OverDrive,
//        DSP_Option::LadderFilter,
//    }};
    
    //This for replaces the manual initialization above, in this case GeneralFilter is added
    //note how interesting is the static_cast option
    for(size_t i = 0; i < static_cast<size_t>(DSP_Option::END_OF_LIST); ++i)
    {
        dspOrder[i] = static_cast<DSP_Option>(i);
    }
    restoreDspOrderFifo.push(dspOrder);
    
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
        
        &overdriveSaturation,
        
        &ladderFilterCutoffHz,
        &ladderFilterResonance,
        &ladderFilterDrive,
        
        &generalFilterFreqHz,
        &generalFilterQuality,
        &generalFilterGain,
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
        
        &getOverdriveSaturationName,
        
        &getLadderFilterCutoffName,
        &getLadderFilterResonanceName,
        &getLadderFilterDriveName,
        
        &getGeneralFilterFreqName,
        &getGeneralFilterQualityName,
        &getGeneralFilterGainName,
    };
    
    
    initCachedParams<juce::AudioParameterFloat*>(floatParams, floatNameFuncs);
    
    auto choiceParams = std::array
    {
        &ladderFilterMode,
        
        &generalFilterMode,
    };
    
    auto choiceFuncs = std::array
    {
        &getLadderFilterModeName,
        
        &getGeneralFilterModeName,
    };
    
    
    initCachedParams<juce::AudioParameterChoice*>(choiceParams, choiceFuncs);
    
    auto bypassParams = std::array
    {
        &phaserBypass,
        &chorusBypass,
        &overdriveBypass,
        &ladderFilterBypass,
        &generalFilterBypass,
    };
    
    auto bypassNameFuncs = std::array
    {
        &getPhaserBypassName,
        &getChorusBypassName,
        &getOverdriveBypassName,
        &getLadderFilterBypassName,
        &getGeneralFilterBypassName,
    };
    
    initCachedParams<juce::AudioParameterBool*>(bypassParams, bypassNameFuncs);
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



void VoxProcessorAudioProcessor::MonoChannelDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    jassert(spec.numChannels == 1);
    
    std::vector<juce::dsp::ProcessorBase*> dsp
    {
        &phaser,
        &chorus,
        &overdrive,
        &ladderFilter,
        &generalFilter,
    };
    
    for (auto p : dsp)
    {
        p->prepare(spec);
        p->reset();
    }
}

void VoxProcessorAudioProcessor::MonoChannelDSP::updateDSPFromParams()
{
    
    phaser.dsp.setRate(p.phaserRateHzSmoother.getCurrentValue());
    phaser.dsp.setCentreFrequency(p.phaserCenterFreqHzSmoother.getCurrentValue());
    phaser.dsp.setDepth(p.phaserDepthPercentSmoother.getCurrentValue());
    phaser.dsp.setFeedback(p.phaserFeedbackPercentSmoother.getCurrentValue());
    phaser.dsp.setMix(p.phaserMixPercentSmoother.getCurrentValue());
    
    chorus.dsp.setRate(p.chorusRateHzSmoother.getCurrentValue());
    chorus.dsp.setDepth(p.chorusDepthPercentSmoother.getCurrentValue());
    chorus.dsp.setCentreDelay(p.chorusCenterDelayMsSmoother.getCurrentValue());
    chorus.dsp.setFeedback(p.chorusFeedbackPercentSmoother.getCurrentValue());
    chorus.dsp.setMix(p.chorusMixPercentSmoother.getCurrentValue());
    
    overdrive.dsp.setDrive(p.overdriveSaturationSmoother.getCurrentValue());
    
    ladderFilter.dsp.setMode(static_cast<juce::dsp::LadderFilterMode>(p.ladderFilterMode->getIndex()));
    ladderFilter.dsp.setCutoffFrequencyHz(p.ladderFilterCutoffHzSmoother.getCurrentValue());
    ladderFilter.dsp.setResonance(p.ladderFilterResonanceSmoother.getCurrentValue());
    ladderFilter.dsp.setDrive(p.ladderFilterDriveSmoother.getCurrentValue());
    
    //update generalFilter coefficients
    //choices: peak, bandpass, notch, allpass
    auto genMode = p.generalFilterMode->getIndex();
    auto genHz = p.generalFilterFreqHzSmoother.getCurrentValue();
    auto genQ = p.generalFilterQualitySmoother.getCurrentValue();
    auto genGain = p.generalFilterGainSmoother.getCurrentValue();
    
    bool filterChanged = false;
    filterChanged |= (filterFreq != genHz);
    filterChanged |= (filterQ != genQ);
    filterChanged |= (filterGain != genGain);
    
    auto updatedMode = static_cast<GeneralFilterMode>(genMode);
    filterChanged |= (filterMode != updatedMode);
    
    if(filterChanged)
    {
        auto sampleRate = p.getSampleRate();
        
        filterMode = updatedMode;
        filterQ = genQ;
        filterFreq = genHz;
        filterGain = genGain;
        
        juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
        switch (filterMode)
        {
            case GeneralFilterMode::Peak:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                                   filterFreq,
                                                                                   filterQ,
                                                                                   juce::Decibels::decibelsToGain(filterGain));
                break;
            }
            case GeneralFilterMode::Bandpass:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate,
                                                                                 filterFreq,
                                                                                 filterQ);
                break;
            }
            case GeneralFilterMode::Notch:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate,
                                                                              filterFreq,
                                                                              filterQ);
                break;
            }
            case GeneralFilterMode::Allpass:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate,
                                                                                filterFreq,
                                                                                filterQ);
                break;
            }
            case GeneralFilterMode::END_OF_LIST:
            {
                jassertfalse;
                break;
            }
        }
        
        if (coefficients != nullptr)
        {
//            if (generalFilter.dsp.coefficients->coefficients.size() != coefficients->coefficients.size())
//            {
//                jassertfalse;
//            }
//            
            *generalFilter.dsp.coefficients = *coefficients;
            generalFilter.reset();
        }
    }
};

void VoxProcessorAudioProcessor::updateSmoothersFromParams(int numSamplesToSkip, SmootherUpdateMode init)
{
    auto paramsNeedingSmoothing = std::array
    {
        phaserRateHz,
        phaserCenterFreqHz,
        phaserDepthPercent,
        phaserFeedbackPercent,
        phaserMixPercent,
        chorusRateHz,
        chorusDepthPercent,
        chorusCenterDelayMs,
        chorusFeedbackPercent,
        chorusMixPercent,
        overdriveSaturation,
        ladderFilterCutoffHz,
        ladderFilterResonance,
        ladderFilterDrive,
        generalFilterFreqHz,
        generalFilterQuality,
        generalFilterGain,
    };
    
    auto smoothers = getSmoothers();
    jassert(paramsNeedingSmoothing.size() == smoothers.size());
    
    for(size_t i = 0; i < smoothers.size(); ++i)
    {
        auto smoother = smoothers[i];
        auto param = paramsNeedingSmoothing[i];
        
        if (init == SmootherUpdateMode::initialize) {
            smoother->setCurrentAndTargetValue(param->get());
        }else{
            smoother->setTargetValue(param->get());
        }
        
        smoother->skip(numSamplesToSkip);
    }
    
}

void VoxProcessorAudioProcessor::MonoChannelDSP::process(juce::dsp::AudioBlock<float> block, const DSP_Order &dspOrder)
{
    //convert dspOrder into an array of pointers to the DSP objects
    DSP_Pointers dspPointers;
    dspPointers.fill({});
    
    //We reasign the objects to their pointers if needed
    for(size_t i = 0; i < dspPointers.size(); ++i)
    {
        switch (dspOrder[i])
        {
            case DSP_Option::Phase:
                dspPointers[i].processor = &phaser;
                dspPointers[i].bypassed = p.phaserBypass->get();
                break;
            case DSP_Option::Chorus:
                dspPointers[i].processor = &chorus;
                dspPointers[i].bypassed = p.chorusBypass->get();
                break;
            case DSP_Option::OverDrive:
                dspPointers[i].processor = &overdrive;
                dspPointers[i].bypassed = p.overdriveBypass->get();
                break;
            case DSP_Option::LadderFilter:
                dspPointers[i].processor = &ladderFilter;
                dspPointers[i].bypassed = p.ladderFilterBypass->get();
                break;
            case DSP_Option::GeneralFilter:
                dspPointers[i].processor = &generalFilter;
                dspPointers[i].bypassed = p.generalFilterBypass->get();
                break;
            case DSP_Option::END_OF_LIST:
                jassertfalse;
                break;
        }
    }
    
    //Now, with the list up to date we can process
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    for(size_t i = 0; i < dspPointers.size(); ++i)
    {
        if(dspPointers[i].processor != nullptr)
        {
            juce::ScopedValueSetter<bool> svs(context.isBypassed, dspPointers[i].bypassed);
#if VERIFY_BYPASS_FUNCTIONALITY
            if( context.isBypassed )
            {
                jassertfalse;
            }
            
            if( dspPointers[i].processor == &generalFilter )
            {
                continue;
            }
#endif
            dspPointers[i].processor->process(context);
        }
    }
}

//==============================================================================
void VoxProcessorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    
    leftChannel.prepare(spec);
    rightChannel.prepare(spec);
    
    for(auto smoother : getSmoothers())
    {
        smoother->reset(sampleRate, 0.005);
    }
    
    updateSmoothersFromParams(1, SmootherUpdateMode::initialize);
}

std::vector<juce::SmoothedValue<float>*> VoxProcessorAudioProcessor::getSmoothers()
{
    auto smoothers = std::vector
    {
        &phaserRateHzSmoother,
        &phaserCenterFreqHzSmoother,
        &phaserDepthPercentSmoother,
        &phaserFeedbackPercentSmoother,
        &phaserMixPercentSmoother,
        &chorusRateHzSmoother,
        &chorusDepthPercentSmoother,
        &chorusCenterDelayMsSmoother,
        &chorusFeedbackPercentSmoother,
        &chorusMixPercentSmoother,
        &overdriveSaturationSmoother,
        &ladderFilterCutoffHzSmoother,
        &ladderFilterResonanceSmoother,
        &ladderFilterDriveSmoother,
        &generalFilterFreqHzSmoother,
        &generalFilterQualitySmoother,
        &generalFilterGainSmoother,
    };
    
    return smoothers;
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
    
    //drive: 1-100
    name = getOverdriveSaturationName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                           name,
                                                           juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f),
                                                           1.f,
                                                           ""));
    
    //mode: LadderFilterMode enum (int)
    name = getLadderFilterModeName();
    auto choices = getLadderFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{name, versionHint},
                                                            name,
                                                            choices,
                                                            0));
    
    name = getLadderFilterCutoffName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                           name,
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 0.1f, 1.f),
                                                           20000.f,
                                                           ""));
    
    name = getLadderFilterResonanceName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                           name,
                                                           juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                           0.f,
                                                           ""));
    
    name = getLadderFilterDriveName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                           name,
                                                           juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f),
                                                           1.f,
                                                           ""));
    
    
    
    //Mode
    name = getGeneralFilterModeName();
    choices = getGenearlFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{name, versionHint},
                                                            name,
                                                            choices,
                                                            0));
    
    //Freq
    name = getGeneralFilterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                           name,
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
                                                           750.f));
    
    //Quality
    name = getGeneralFilterQualityName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                           name,
                                                           juce::NormalisableRange<float>(1.f, 10.f, 0.05f, 1.f),
                                                           1.f));
    
    //Gain
    name = getGeneralFilterGainName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionHint},
                                                           name,
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));
    
    name = getPhaserBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{name, versionHint}, name, false));
    
    name = getChorusBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{name, versionHint}, name, false));
    
    name = getOverdriveBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{name, versionHint}, name, false));
    
    name = getLadderFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{name, versionHint}, name, false));
    
    name = getGeneralFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{name, versionHint}, name, false));
    
    return layout;
}

void VoxProcessorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    //[DONE]: add APVTS
    //[DONE]: create audio parameters for all dsp choices
    //[DONE]]: update DSP here from audio parameters
    //[DONE]: save/load settings
    //[DONE]: save/load dsp order
    //[DONE]: filters are mono, not stereo
    //[DONE]: update generalFilter coefficients
    //[DONE]: add smoothers for all param updates
    //[DONE]: save/load DSP order
    //[DONE]: Drag-To-Reorder GUI
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
    
    
    leftChannel.updateDSPFromParams();
    rightChannel.updateDSPFromParams();
    
    //Temp instance to pull into
    auto newDSPOrder = DSP_Order(); // <-- This is just an array
    
    //Try to pull from the Fifo
    while(dspOrderFifo.pull(newDSPOrder))
    {
#if VERIFY_BYPASS_FUNCTIONALITY
        jassertfalse;
#endif
    }
    //If pull succeeded, we refresh dspOrder
    if(newDSPOrder != DSP_Order())
        dspOrder = newDSPOrder;
    
//    auto block = juce::dsp::AudioBlock<float>(buffer);
//    leftChannel.process(block.getSingleChannelBlock(0), dspOrder);
//    rightChannel.process(block.getSingleChannelBlock(1), dspOrder);
    
    auto samplesRemaining = buffer.getNumSamples();
    auto maxSamplesToProcess = juce::jmin(samplesRemaining, 64);
    
    auto block = juce::dsp::AudioBlock<float>(buffer);
    size_t startSample = 0;
    while (samplesRemaining > 0)
    {
        auto samplesToProcess = juce::jmin(samplesRemaining, maxSamplesToProcess);
        updateSmoothersFromParams(samplesToProcess, SmootherUpdateMode::liveInRealTime);
        
        leftChannel.updateDSPFromParams();
        rightChannel.updateDSPFromParams();
        
        auto subBlock = block.getSubBlock(startSample, samplesToProcess);
        leftChannel.process(subBlock.getSingleChannelBlock(0), dspOrder);
        rightChannel.process(subBlock.getSingleChannelBlock(1), dspOrder);
        
        startSample += samplesToProcess;
        samplesRemaining -= samplesToProcess;
    }
}

//==============================================================================
bool VoxProcessorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VoxProcessorAudioProcessor::createEditor()
{
//    return new juce::GenericAudioProcessorEditor(*this);
    return new VoxProcessorAudioProcessorEditor (*this);
}

template<>
struct juce::VariantConverter<VoxProcessorAudioProcessor::DSP_Order>
{
    static VoxProcessorAudioProcessor::DSP_Order fromVar(const juce::var& v)
    {
        using T = VoxProcessorAudioProcessor::DSP_Order;
        T dspOrder;
        
        jassert(v.isBinaryData());
        if(!v.isBinaryData())
        {
            dspOrder.fill(VoxProcessorAudioProcessor::DSP_Option::END_OF_LIST);
        }
        else
        {
            auto mb = *v.getBinaryData();
            
            juce::MemoryInputStream mis(mb, false);
            std::vector<int> arr;
            while(!mis.isExhausted())
            {
                arr.push_back(mis.readInt());
            }
            jassert(arr.size() == dspOrder.size());
            
            for(size_t i = 0; i < dspOrder.size(); ++i)
            {
                dspOrder[i] = static_cast<VoxProcessorAudioProcessor::DSP_Option>(arr[i]);
            }
        }
        return dspOrder;
    }
    
    static juce::var toVar( const VoxProcessorAudioProcessor::DSP_Order& t)
    {
        juce::MemoryBlock mb;
        //juce MOS uses scoping to complete writing to the memory block correctly.
        {
            juce::MemoryOutputStream mos(mb, false);
            
            for( auto& v : t)
            {
                mos.writeInt(static_cast<int>(v));
            }
        }
        
        return mb;
    }
};


//==============================================================================
void VoxProcessorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    apvts.state.setProperty("dspOrder",
                            juce::VariantConverter<VoxProcessorAudioProcessor::DSP_Order>::toVar(dspOrder),
                            nullptr);
    
    juce::MemoryOutputStream mos(destData, false);
    apvts.state.writeToStream(mos);

}

void VoxProcessorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if(tree.isValid())
    {
        apvts.replaceState(tree);
        if( apvts.state.hasProperty("dspOrder"))
        {
            auto order = juce::VariantConverter<VoxProcessorAudioProcessor::DSP_Order>::fromVar(apvts.state.getProperty("dspOrder"));
            dspOrderFifo.push(order);
            restoreDspOrderFifo.push(order);
        }
        DBG(apvts.state.toXmlString());
        
#if VERIFY_BYPASS_FUNCTIONALITY
        juce::Timer::callAfterDelay(1000,[this]()
        {
            DSP_Order order;
            order.fill(DSP_Option::LadderFilter);
            order[0] = DSP_Option::Chorus;
            
            //bypass the Chorus
            chorusBypass->setValueNotifyingHost(1.f);
            dspOrderFifo.push(order);
        });
#endif
        
    }
}

std::vector<juce::RangedAudioParameter*> VoxProcessorAudioProcessor::getParamsForOption(VoxProcessorAudioProcessor::DSP_Option option)
{
    switch (option)
    {
        case DSP_Option::Phase:
        {
            return
            {
                phaserRateHz,
                phaserCenterFreqHz,
                phaserDepthPercent,
                phaserFeedbackPercent,
                phaserMixPercent,
                phaserBypass,
            };
        }
        case DSP_Option::Chorus:
        {
            return
            {
                chorusRateHz,
                chorusDepthPercent,
                chorusCenterDelayMs,
                chorusFeedbackPercent,
                chorusMixPercent,
                chorusBypass,
            };
        }
        case DSP_Option::OverDrive:
        {
            return
            {
                overdriveSaturation,
                overdriveBypass,
            };
        }
        case DSP_Option::LadderFilter:
        {
            return
            {
                ladderFilterMode,
                ladderFilterCutoffHz,
                ladderFilterResonance,
                ladderFilterDrive,
                ladderFilterBypass,
            };
        }
        case DSP_Option::GeneralFilter:
        {
            return
            {
                generalFilterMode,
                generalFilterFreqHz,
                generalFilterQuality,
                generalFilterGain,
                generalFilterBypass,
            };
        }
        case DSP_Option::END_OF_LIST:
            break;
    }
    
    jassertfalse;
    return {};
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VoxProcessorAudioProcessor();
}
