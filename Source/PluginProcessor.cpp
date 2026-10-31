#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::AudioProcessorValueTreeState::ParameterLayout makeParams() {
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    p.push_back(std::make_unique<AudioParameterFloat>("cutoff", "Cutoff", NormalisableRange<float>(80.f, 12000.f, 1.f, .35f), 1200.f));
    p.push_back(std::make_unique<AudioParameterFloat>("resonance", "Resonance", 0.f, .97f, .72f));
    p.push_back(std::make_unique<AudioParameterFloat>("env", "Env Mod", 0.f, 1.f, .55f));
    p.push_back(std::make_unique<AudioParameterFloat>("decay", "Decay", .05f, 2.f, .32f));
    p.push_back(std::make_unique<AudioParameterFloat>("accent", "Accent", 0.f, 1.f, .7f));
    p.push_back(std::make_unique<AudioParameterFloat>("wave", "Wave (0=saw, 1=square)", 0.f, 1.f, 0.f));
    p.push_back(std::make_unique<AudioParameterBool>("sequencer", "Host Sync Sequencer", true));
    p.push_back(std::make_unique<AudioParameterBool>("playing", "Pattern Play", true));
    p.push_back(std::make_unique<AudioParameterFloat>("tempo", "Internal Tempo", 40.f, 240.f, 120.f));
    for (int i = 0; i < 16; ++i) {
        auto n = juce::String(i + 1);
        p.push_back(std::make_unique<AudioParameterInt>("step" + n, "Step " + n + " Note (0=rest)", 0, 24, (i % 4 == 0) ? 12 : 0));
        p.push_back(std::make_unique<AudioParameterBool>("accent" + n, "Step " + n + " Accent", false));
        p.push_back(std::make_unique<AudioParameterBool>("slide" + n, "Step " + n + " Slide", false));
    }
    return {p.begin(), p.end()};
}
Acid303AudioProcessor::Acid303AudioProcessor() : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)), state(*this, nullptr, "PARAMETERS", makeParams()) {}
void Acid303AudioProcessor::prepareToPlay(double sr, int) { sampleRate=sr; voice={}; currentStep=-1; }
void Acid303AudioProcessor::processBlock(juce::AudioBuffer<float>& b, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals; b.clear();
    auto* cutoff=state.getRawParameterValue("cutoff"); auto* res=state.getRawParameterValue("resonance");
    auto* envAmt=state.getRawParameterValue("env"); auto* decay=state.getRawParameterValue("decay"); auto* accent=state.getRawParameterValue("accent"); auto* wave=state.getRawParameterValue("wave");
    double bpm=120.0, ppq=beatPosition; bool playing=false;
    bool hostPlaying=false;
    if (auto* ph=getPlayHead()) if (auto pos=ph->getPosition()) { if (auto t=pos->getBpm()) bpm=*t; if (auto q=pos->getPpqPosition()) ppq=*q; hostPlaying=pos->getIsPlaying(); }
    const bool sync=state.getRawParameterValue("sequencer")->load() > .5f;
    const bool patternOn=state.getRawParameterValue("playing")->load() > .5f;
    if (!sync) bpm=state.getRawParameterValue("tempo")->load();
    playing=sync ? hostPlaying : patternOn;
    const double stepBeats=.25;
    const double inc=bpm/(60.0*sampleRate);
    auto event=midi.begin();
    for (int i=0;i<b.getNumSamples();++i) {
        if (sync && playing) {
            int step=(int)std::floor((ppq + inc*i)/stepBeats);
            if (step!=currentStep) { currentStep=step; int s=((step%16)+16)%16; auto idx=juce::String(s+1); int note=(int)state.getRawParameterValue("step"+idx)->load();
                bool slide=state.getRawParameterValue("slide"+idx)->load()>.5f;
                if (note>0) { int midiNote=36+(note-1); if (!slide || voice.note<0) { voice.phase=0; voice.env=1.f; voice.filter=0; } voice.note=midiNote; }
                else if (!slide) voice.note=-1;
            }
        } else currentStep=-1;
        while(event!=midi.end() && (*event).samplePosition<=i) { auto m=(*event).getMessage(); if(m.isNoteOn()) { voice.note=m.getNoteNumber(); voice.phase=0; voice.env=1.f; } if(m.isNoteOff() && voice.note==m.getNoteNumber()) voice.note=-1; ++event; }
        float out=0;
        if(voice.note>=0 || voice.env>0.0001f) {
            double hz=voice.note>=0 ? juce::MidiMessage::getMidiNoteInHertz(voice.note) : 0.0;
            voice.phase+=hz/sampleRate; if(voice.phase>=1) voice.phase-=1;
            float saw=(float)(2.0*voice.phase-1.0), sq=voice.phase<.5?1.f:-1.f;
            float osc=juce::jmap(wave->load(),saw,sq);
            float env=voice.env;
            bool stepAccent=currentStep>=0 && state.getRawParameterValue("accent"+juce::String((currentStep%16)+1))->load()>.5f;
            float accentGain=stepAccent ? (1.f+accent->load()) : 1.f;
            float fc=juce::jlimit(80.f, 18000.f, cutoff->load()+envAmt->load()*env*8000.f);
            float f=1.f-std::exp(-juce::MathConstants<float>::twoPi*fc/(float)sampleRate);
            // Resonant one-pole feedback filter: classic squelchy subtractive character.
            float input=osc + voice.resonanceState*res->load()*.92f;
            voice.filter += f*(input-voice.filter);
            voice.resonanceState=voice.filter-osc*.18f;
            out=voice.filter*0.22f*accentGain;
            voice.env*=std::exp(-1.f/(decay->load()*(float)sampleRate));
        }
        for(int ch=0;ch<b.getNumChannels();++ch) b.setSample(ch,i,out);
    }
    if (sync && playing) beatPosition=ppq+inc*b.getNumSamples(); else beatPosition+=inc*b.getNumSamples();
}
juce::AudioProcessorEditor* Acid303AudioProcessor::createEditor(){ return new Acid303AudioProcessorEditor(*this); }
void Acid303AudioProcessor::getStateInformation(juce::MemoryBlock& d){ auto x=state.copyState(); std::unique_ptr<juce::XmlElement> xml(x.createXml()); copyXmlToBinary(*xml,d); }
void Acid303AudioProcessor::setStateInformation(const void* d,int n){ std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(d,n)); if(xml && xml->hasTagName(state.state.getType())) state.replaceState(juce::ValueTree::fromXml(*xml)); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){ return new Acid303AudioProcessor(); }
