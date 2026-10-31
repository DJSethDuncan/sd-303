#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::AudioProcessorValueTreeState::ParameterLayout makeParams() {
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    auto percent=AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){ return String(juce::roundToInt(v*100.f))+" %"; }).withValueFromStringFunction([](const String& text){ return text.getFloatValue()/100.f; });
    NormalisableRange<float> cutoffRange(30.f,18000.f,1.f); cutoffRange.setSkewForCentre(1000.f);
    p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"cutoff",1},"Cutoff",cutoffRange,1200.f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){ return String(juce::roundToInt(v))+" Hz"; })));
    p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"resonance",1},"Resonance",NormalisableRange<float>(0.f,.97f),.55f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){ return String(juce::roundToInt(v/.97f*100.f))+" %"; }).withValueFromStringFunction([](const String& text){ return text.getFloatValue()*.0097f; })));
    p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"env",1},"Envelope Modulation",NormalisableRange<float>(0.f,1.f),.45f,percent));
    NormalisableRange<float> decayRange(.05f,2.f,.001f); decayRange.setSkewForCentre(.4f);
    p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"decay",1},"Decay",decayRange,.32f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){ return String(juce::roundToInt(v*1000.f))+" ms"; }).withValueFromStringFunction([](const String& text){ return text.getFloatValue()/1000.f; })));
    p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"accent",1},"Accent",NormalisableRange<float>(0.f,1.f),.7f,percent));
    p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"wave",1},"Waveform",NormalisableRange<float>(0.f,1.f,1.f),0.f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){ return v<.5f?String("SAW"):String("SQUARE"); }).withValueFromStringFunction([](const String& text){ return text.containsIgnoreCase("square") || text.getFloatValue()>=.5f ? 1.f : 0.f; })));
    p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"volume",1},"Volume",NormalisableRange<float>(0.f,1.f),.65f,percent));
    p.push_back(std::make_unique<AudioParameterBool>("sequencer","Host Tempo",true));
    p.push_back(std::make_unique<AudioParameterBool>("playing","Pattern Run",false));
    p.push_back(std::make_unique<AudioParameterInt>("tempo","Internal Tempo",40,240,120));
    for(int i=0;i<16;++i) {
        auto n=String(i+1);
        p.push_back(std::make_unique<AudioParameterInt>("step"+n,"Step "+n+" Note (0=rest)",0,24,(i%4==0)?1:0));
        p.push_back(std::make_unique<AudioParameterBool>("accent"+n,"Step "+n+" Accent",false));
        p.push_back(std::make_unique<AudioParameterBool>("slide"+n,"Step "+n+" Slide Into",false));
    }
    return {p.begin(),p.end()};
}
Acid303AudioProcessor::Acid303AudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"PARAMETERS",makeParams()) {
    cutoffParam=state.getRawParameterValue("cutoff"); resParam=state.getRawParameterValue("resonance"); envParam=state.getRawParameterValue("env");
    decayParam=state.getRawParameterValue("decay"); accentParam=state.getRawParameterValue("accent"); waveParam=state.getRawParameterValue("wave"); volumeParam=state.getRawParameterValue("volume");
    for(int i=0;i<16;++i) { auto n=juce::String(i+1); notes[i]=state.getRawParameterValue("step"+n); accents[i]=state.getRawParameterValue("accent"+n); slides[i]=state.getRawParameterValue("slide"+n); }
}
void Acid303AudioProcessor::prepareToPlay(double sr,int) {
    sampleRate=sr; voice={}; currentStep=-1; beatPosition=0; wasPlaying=false; midiHeld=false; auditionSamples=0; playingStep.store(-1);
    auto reset=[sr](auto& smooth,float value){ smooth.reset(sr,.025); smooth.setCurrentAndTargetValue(value); };
    reset(cutoffSmooth,cutoffParam->load()); reset(resSmooth,resParam->load()); reset(envSmooth,envParam->load());
    reset(decaySmooth,decayParam->load()); reset(accentSmooth,accentParam->load()); reset(volumeSmooth,volumeParam->load());
}
// Band-limited discontinuity correction for saw and pulse edges.
static double polyBlep(double t,double dt) {
    if(t<dt) { t/=dt; return t+t-t*t-1.; }
    if(t>1.-dt) { t=(t-1.)/dt; return t*t+t+t+1.; }
    return 0.;
}
void Acid303AudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals; b.clear();
    const bool playing=state.getRawParameterValue("playing")->load()>.5f;
    const bool sync=state.getRawParameterValue("sequencer")->load()>.5f;
    double bpm=state.getRawParameterValue("tempo")->load(),ppq=beatPosition;
    bool hostClock=false;
    if(sync) if(auto* ph=getPlayHead()) if(auto pos=ph->getPosition()) {
        if(pos->getIsPlaying() && pos->getPpqPosition()) { hostClock=true; ppq=*pos->getPpqPosition(); if(auto t=pos->getBpm()) bpm=*t; }
    }
    const bool restart=restartRequested.exchange(false);
    if(restart || (playing && !wasPlaying)) {
        beatPosition=0; currentStep=-1; if(!hostClock) ppq=0;
        voice.gate=false; auditionSamples=0;
    }
    if(!playing && wasPlaying && !midiHeld) voice.gate=false;
    wasPlaying=playing;
    const int preview=auditionRequested.exchange(-1);
    auto trigger=[this](int note,bool slide,float emphasis){
        const bool legato=slide && voice.gate;
        voice.note=note; voice.glide=legato; voice.emphasis=emphasis;
        if(!legato) { voice.pitch=note; voice.env=1.f; }
        voice.gate=true;
    };
    if(preview>=0 && !playing) { trigger(preview,false,0); auditionSamples=(int)(sampleRate*.25); }
    std::array<int,16> pattern{}; std::array<bool,16> acc{},slide{};
    for(int i=0;i<16;++i) { pattern[i]=(int)notes[i]->load(); acc[i]=accents[i]->load()>.5f; slide[i]=slides[i]->load()>.5f; }
    cutoffSmooth.setTargetValue(cutoffParam->load()); resSmooth.setTargetValue(resParam->load()); envSmooth.setTargetValue(envParam->load());
    decaySmooth.setTargetValue(decayParam->load()); accentSmooth.setTargetValue(accentParam->load()); volumeSmooth.setTargetValue(volumeParam->load());
    const double inc=bpm/(60.*sampleRate);
    const float release=std::exp(-1.f/(.012f*(float)sampleRate)),attack=1.f-std::exp(-1.f/(.002f*(float)sampleRate));
    const double glideCoeff=1.-std::exp(-1./(.02*sampleRate));
    auto event=midi.begin();
    for(int i=0;i<b.getNumSamples();++i) {
        if(playing) {
            const int step=(int)std::floor((ppq+inc*i)*4.);
            if(step!=currentStep) { currentStep=step; const int s=((step%16)+16)%16;
                if(pattern[s]>0) trigger(35+pattern[s],slide[s],acc[s]?1.f:0.f);
                else voice.gate=false;
            }
        } else currentStep=-1;
        while(event!=midi.end() && (*event).samplePosition<=i) {
            auto m=(*event).getMessage();
            if(m.isNoteOn()) { midiHeld=true; trigger(m.getNoteNumber(),voice.gate,m.getVelocity()>=100?1.f:0.f); }
            if(m.isNoteOff() && m.getNoteNumber()==voice.note) { midiHeld=false; voice.gate=false; }
            if(m.isAllNotesOff() || m.isAllSoundOff()) { midiHeld=false; voice.gate=false; }
            ++event;
        }
        if(auditionSamples>0 && --auditionSamples==0 && !midiHeld) voice.gate=false;
        voice.pitch+=glideCoeff*((double)voice.note-voice.pitch);
        const float decay=decaySmooth.getNextValue();
        voice.env*=std::exp(-1.f/(decay*(float)sampleRate));
        const float target=voice.gate?std::sqrt(voice.env):0.f;
        if(target>voice.amp) voice.amp+=attack*(target-voice.amp); else voice.amp=target+(voice.amp-target)*release;
        const float emphasis=voice.emphasis*accentSmooth.getNextValue();
        const float cutoff=cutoffSmooth.getNextValue();
        // Octave modulation keeps minimum cutoff meaningful instead of adding a fixed 8 kHz offset.
        const float fc=juce::jlimit(20.f,(float)juce::jmin(18000.,sampleRate*.4),cutoff*std::exp2(envSmooth.getNextValue()*voice.env*(4.f+emphasis*1.5f)));
        const float q=.5f+17.5f*std::pow(resSmooth.getNextValue()/.97f,2.f);
        const float g=std::tan(juce::MathConstants<float>::pi*fc/(float)sampleRate);
        const float k=1.f/q, a1=1.f/(1.f+g*(g+k)), a2=g*a1,a3=g*a2;
        const double dt=juce::jlimit(.000001,.45,juce::MidiMessage::getMidiNoteInHertz((int)std::floor(voice.pitch))*std::exp2((voice.pitch-std::floor(voice.pitch))/12.)/sampleRate);
        const double t=voice.phase;
        double osc=2.*t-1.-polyBlep(t,dt);
        if(waveParam->load()>=.5f) osc=(t<.5?1.:-1.)+polyBlep(t,dt)-polyBlep(std::fmod(t+.5,1.),dt);
        voice.phase=std::fmod(t+dt,1.);
        const float input=std::tanh((float)osc*1.4f)*voice.amp;
        const float v3=input-voice.ic2, v1=a1*voice.ic1+a2*v3, v2=voice.ic2+a2*voice.ic1+a3*v3;
        voice.ic1=2.f*v1-voice.ic1; voice.ic2=2.f*v2-voice.ic2;
        const float f=1.f-std::exp(-juce::MathConstants<float>::twoPi*fc/(float)sampleRate);
        voice.pole+=f*(v2-voice.pole);
        const float out=std::tanh(voice.pole*.6f*(1.f+emphasis*.6f))*volumeSmooth.getNextValue();
        for(int ch=0;ch<b.getNumChannels();++ch) b.setSample(ch,i,out);
    }
    playingStep.store(playing?((currentStep%16)+16)%16:-1,std::memory_order_relaxed);
    if(playing) beatPosition=ppq+inc*b.getNumSamples();
}
juce::AudioProcessorEditor* Acid303AudioProcessor::createEditor(){ return new Acid303AudioProcessorEditor(*this); }
void Acid303AudioProcessor::getStateInformation(juce::MemoryBlock& d){ auto xml=state.copyState().createXml(); copyXmlToBinary(*xml,d); }
void Acid303AudioProcessor::setStateInformation(const void* d,int n){ auto xml=getXmlFromBinary(d,n); if(xml && xml->hasTagName(state.state.getType())) state.replaceState(juce::ValueTree::fromXml(*xml)); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){ return new Acid303AudioProcessor(); }
