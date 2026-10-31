#include "PluginProcessor.h"
#include <iostream>
#include <stdexcept>

static void check(bool ok,const char* message){ if(!ok) throw std::runtime_error(message); }
static void set(Acid303AudioProcessor& p,const char* id,float v){ auto* a=p.state.getParameter(id); a->setValueNotifyingHost(a->convertTo0to1(v)); }
static double render(Acid303AudioProcessor& p,int blocks,juce::MidiBuffer midi={}) {
    double sum=0; juce::AudioBuffer<float> b(2,256);
    for(int n=0;n<blocks;++n){ p.processBlock(b,midi); midi.clear();
        for(int i=0;i<256;++i){ const float x=b.getSample(0,i); check(std::isfinite(x) && std::abs(x)<=1.f,"Audio must remain finite and bounded"); sum+=x*x; }
    }
    return std::sqrt(sum/(blocks*256));
}
struct TestPlayHead : juce::AudioPlayHead {
    bool running=false; double ppq=0, bpm=90;
    juce::Optional<PositionInfo> getPosition() const override {
        PositionInfo info; info.setIsPlaying(running); info.setPpqPosition(ppq); info.setBpm(bpm); return info;
    }
};
int main(){
    juce::ScopedJuceInitialiser_GUI init;
    try {
        Acid303AudioProcessor p; p.prepareToPlay(48000,256);
        check(p.state.getRawParameterValue("playing")->load()==0,"New instance must start stopped");
        check(render(p,20)==0,"Stopped startup must be silent");
        set(p,"sequencer",0); set(p,"playing",1);
        check(render(p,1)>0.00001,"RUN must produce audio from default pattern");
        check(p.getPlayingStep()==0,"RUN must begin at step 1");
        render(p,24); check(p.getPlayingStep()==1,"Internal clock must advance at 120 BPM");
        p.requestPatternRestart(); render(p,1); check(p.getPlayingStep()==0,"RESTART must return to step 1");
        set(p,"playing",0); render(p,200); check(render(p,20)<0.00001,"STOP must release audio");
        check(p.getPlayingStep()==-1,"STOP must clear playback indicator");
        p.auditionNote(48); check(render(p,10)>0.001,"Piano must audition while stopped");
        render(p,200); check(render(p,10)<0.00001,"Audition must release automatically");
        juce::MidiBuffer midi; midi.addEvent(juce::MidiMessage::noteOn(1,48,(juce::uint8)100),0);
        check(render(p,5,midi)>0.001,"MIDI note must sound");
        midi.clear(); midi.addEvent(juce::MidiMessage::noteOff(1,48),0); render(p,200,midi);
        check(render(p,10)<0.00001,"MIDI note off must release");
        set(p,"step2",15); set(p,"accent2",1); set(p,"slide2",1);
        juce::MemoryBlock state; p.getStateInformation(state);
        Acid303AudioProcessor restored; restored.setStateInformation(state.getData(),(int)state.getSize());
        check(restored.state.getRawParameterValue("step2")->load()==15 && restored.state.getRawParameterValue("slide2")->load()==1,"Pattern must survive state recall");
        check(p.state.getParameter("resonance")->getText(1,0)=="100 %","Resonance format");
        check(p.state.getParameter("wave")->getText(1,0)=="SQUARE","Waveform format");
        check(std::abs(p.state.getParameter("env")->getValueForText("70 %")-.7f)<.001f,"Percent parsing");
        set(p,"tempo",123.4f);
        check(p.state.getRawParameterValue("tempo")->load()==123,"Tempo must be whole BPM");
        TestPlayHead host;
        Acid303AudioProcessor synced; synced.setPlayHead(&host); synced.prepareToPlay(48000,256);
        host.running=true; host.ppq=1.5; set(synced,"playing",1); render(synced,1);
        check(synced.getPlayingStep()==6,"Playing host position must drive sequencer");
        host.ppq=0; render(synced,1); check(synced.getPlayingStep()==0,"Host loop must return to first step");
        host.running=false; set(synced,"tempo",120); synced.requestPatternRestart(); render(synced,25);
        check(synced.getPlayingStep()==1,"Stopped host must fall back to internal clock");
        set(synced,"playing",0); render(synced,1); set(synced,"playing",1); render(synced,1);
        check(synced.getPlayingStep()==0,"RUN after STOP must restart at first step");
        synced.setPlayHead(nullptr);
        // Compare equal renders after resetting engine: envelope off isolates filter sweep.
        auto tone=[](float cutoff,float resonance){
            Acid303AudioProcessor synth; set(synth,"env",0); set(synth,"cutoff",cutoff); set(synth,"resonance",resonance);
            synth.prepareToPlay(48000,256); juce::MidiBuffer m; m.addEvent(juce::MidiMessage::noteOn(1,48,(juce::uint8)80),0);
            return render(synth,40,m);
        };
        auto low=tone(30,0),high=tone(12000,0),flat=tone(260,0),peak=tone(260,.97f);
        check(high>low*4,"Cutoff sweep must measurably remove harmonics and fundamental");
        check(peak>flat*1.5,"Resonance must produce a strong peak near note frequency");
        std::cout<<"Filter RMS: low="<<low<<" open="<<high<<" flat="<<flat<<" resonant="<<peak<<"\n";
        for(double sr : {22050.,44100.,48000.,96000.}) {
            Acid303AudioProcessor synth; synth.prepareToPlay(sr,256); set(synth,"playing",1);
            for(int i=0;i<80;++i){ set(synth,"cutoff",i%2?30.f:18000.f); set(synth,"resonance",i%3?.97f:0.f); set(synth,"env",i%2?1.f:0.f); render(synth,4); }
        }
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        editor->setVisible(true);
        auto* blackKey=dynamic_cast<juce::TextButton*>(editor->getComponentAt(240,600));
        check(blackKey && blackKey->getButtonText()=="C#2","Black keys must remain above adjacent white keys");
        const auto image=editor->createComponentSnapshot(editor->getLocalBounds());
        juce::File file(juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("sd303-ux-panel.png"));
        auto stream=file.createOutputStream();
        check(stream!=nullptr,"Snapshot file must open"); stream->setPosition(0); stream->truncate();
        juce::PNGImageFormat png; check(png.writeImageToStream(image,*stream),"Panel snapshot must render");
        std::cout<<"Panel snapshot: "<<file.getFullPathName()<<"\n";
        std::cout<<"PASS: startup, transport, audition, MIDI, recall, formatting, filter response, extreme sweeps\n";
    } catch(const std::exception& e){ std::cerr<<"FAIL: "<<e.what()<<"\n"; return 1; }
}
