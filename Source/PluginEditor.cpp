#include "PluginEditor.h"
Acid303AudioProcessorEditor::Acid303AudioProcessorEditor(Acid303AudioProcessor& p):AudioProcessorEditor(p),processor(p){
    setSize(560,330); title.setText("ACID 303",juce::dontSendNotification); title.setJustificationType(juce::Justification::centred); title.setFont(juce::Font(30.f,juce::Font::bold)); addAndMakeVisible(title);
    juce::Slider* sliders[]={&cutoff,&resonance,&env,&decay,&accent,&wave}; const char* names[]={"CUTOFF","RESONANCE","ENV MOD","DECAY","ACCENT","WAVE"}; const char* ids[]={"cutoff","resonance","env","decay","accent","wave"};
    std::unique_ptr<Attachment>* at[]={&cutoffA,&resA,&envA,&decayA,&accentA,&waveA};
    for(int i=0;i<6;++i){ auto* s=sliders[i]; s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,72,20); addAndMakeVisible(*s); s->setName(names[i]); *at[i]=std::make_unique<Attachment>(processor.state,ids[i],*s); }
    addAndMakeVisible(sequencer); seqA=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,"sequencer",sequencer);
}
void Acid303AudioProcessorEditor::paint(juce::Graphics& g){ g.fillAll(juce::Colour(0xff17191a)); g.setColour(juce::Colour(0xffc7e54b)); g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(8),12); g.setColour(juce::Colour(0xff17191a)); g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(12),9); g.setColour(juce::Colour(0xffc7e54b)); g.setFont(12); g.drawText("CLASSIC ACID SYNTH  •  TEMPO-LOCKED 16TH PATTERN",12,46,getWidth()-24,20,juce::Justification::centred); }
void Acid303AudioProcessorEditor::resized(){ title.setBounds(0,12,getWidth(),38); int w=getWidth()/3; juce::Slider* s[]={&cutoff,&resonance,&env,&decay,&accent,&wave}; for(int i=0;i<6;++i)s[i]->setBounds((i%3)*w+8,76+(i/3)*112,w-16,102); sequencer.setBounds(170,302,220,24); }
