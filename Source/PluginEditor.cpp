#include "PluginEditor.h"

Acid303AudioProcessorEditor::Acid303AudioProcessorEditor(Acid303AudioProcessor& p) : AudioProcessorEditor(p), proc(p) {
    setSize(700, 470);
    title.setText("SD-303", juce::dontSendNotification); title.setJustificationType(juce::Justification::centred); title.setFont(juce::Font(30.f, juce::Font::bold)); addAndMakeVisible(title);
    stepLabel.setJustificationType(juce::Justification::centredLeft); addAndMakeVisible(stepLabel);
    juce::Slider* sliders[]={&cutoff,&resonance,&env,&decay,&accent,&wave,&tempo};
    const char* names[]={"CUTOFF","RESONANCE","ENV MOD","DECAY","ACCENT","WAVE","TEMPO"};
    const char* ids[]={"cutoff","resonance","env","decay","accent","wave","tempo"};
    std::unique_ptr<Attachment>* at[]={&cutoffA,&resA,&envA,&decayA,&accentA,&waveA,&tempoA};
    for(int i=0;i<7;++i){ auto* s=sliders[i]; s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,18); s->setName(names[i]); addAndMakeVisible(*s); *at[i]=std::make_unique<Attachment>(proc.state,ids[i],*s); }
    addAndMakeVisible(sequencer); addAndMakeVisible(playing); addAndMakeVisible(stepAccent); addAndMakeVisible(slide);
    seqA=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.state,"sequencer",sequencer);
    playingA=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.state,"playing",playing);
    note.setRange(0,24,1); note.setSliderStyle(juce::Slider::IncDecButtons); note.setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,22); addAndMakeVisible(note);
    for(int i=0;i<16;++i){ steps[i].setButtonText(juce::String(i+1)); steps[i].onClick=[this,i]{ selectedStep=i; updateStepControls(); }; addAndMakeVisible(steps[i]); }
    note.onValueChange=[this]{ auto* param=proc.state.getParameter("step"+juce::String(selectedStep+1)); if(param) param->setValueNotifyingHost(param->convertTo0to1((float)note.getValue())); };
    stepAccent.onClick=[this]{ auto* param=proc.state.getParameter("accent"+juce::String(selectedStep+1)); if(param) param->setValueNotifyingHost(stepAccent.getToggleState()?1.f:0.f); };
    slide.onClick=[this]{ auto* param=proc.state.getParameter("slide"+juce::String(selectedStep+1)); if(param) param->setValueNotifyingHost(slide.getToggleState()?1.f:0.f); };
    updateStepControls();
}
void Acid303AudioProcessorEditor::updateStepControls(){
    auto suffix=juce::String(selectedStep+1);
    note.setValue(proc.state.getRawParameterValue("step"+suffix)->load(),juce::dontSendNotification);
    stepAccent.setToggleState(proc.state.getRawParameterValue("accent"+suffix)->load()>.5f,juce::dontSendNotification);
    slide.setToggleState(proc.state.getRawParameterValue("slide"+suffix)->load()>.5f,juce::dontSendNotification);
    stepLabel.setText("STEP "+suffix+"   NOTE (0 = REST, 1–24 = PITCH)",juce::dontSendNotification);
    for(int i=0;i<16;++i){ int v=(int)proc.state.getRawParameterValue("step"+juce::String(i+1))->load(); steps[i].setColour(juce::TextButton::buttonColourId,i==selectedStep?juce::Colour(0xffc7e54b):(v?juce::Colour(0xff53612b):juce::Colour(0xff35383a))); steps[i].setColour(juce::TextButton::textColourOffId,juce::Colours::white); }
}
void Acid303AudioProcessorEditor::paint(juce::Graphics& g){ g.fillAll(juce::Colour(0xff17191a)); g.setColour(juce::Colour(0xffc7e54b)); g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(8),12,2); g.setColour(juce::Colour(0xffc7e54b)); g.setFont(12); g.drawText("PROGRAMMABLE 16-STEP ACID SEQUENCER",18,48,getWidth()-36,20,juce::Justification::centred); }
void Acid303AudioProcessorEditor::resized(){
    title.setBounds(0,8,getWidth(),38); sequencer.setBounds(28,68,140,25); playing.setBounds(178,68,150,25); tempo.setBounds(550,57,112,92);
    int x=28; for(int i=0;i<16;++i){ steps[i].setBounds(x+(i%8)*80,108+(i/8)*43,66,32); }
    stepLabel.setBounds(28,202,300,24); note.setBounds(300,191,110,45); stepAccent.setBounds(420,202,100,24); slide.setBounds(525,202,145,24);
    juce::Slider* s[]={&cutoff,&resonance,&env,&decay,&accent,&wave}; for(int i=0;i<6;++i)s[i]->setBounds(18+i*112,267,106,130);
}
