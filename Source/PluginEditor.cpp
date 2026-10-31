#include "PluginEditor.h"

void SD303LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) {
    auto r=juce::jmin(w,h)*0.72f; auto cx=x+w*0.5f, cy=y+h*0.45f; auto bounds=juce::Rectangle<float>(cx-r*0.5f,cy-r*0.5f,r,r);
    g.setColour(juce::Colour(0xff222326)); g.fillEllipse(bounds.translated(0,2));
    g.setColour(juce::Colour(0xff424448)); g.fillEllipse(bounds);
    g.setColour(juce::Colour(0xfff17b32)); g.drawEllipse(bounds.reduced(2),2.0f);
    g.setColour(juce::Colour(0xffd7d1c3)); g.fillEllipse(bounds.reduced(r*0.16f));
    g.setColour(juce::Colour(0xff26272a)); g.fillEllipse(bounds.reduced(r*0.26f));
    auto a=start+pos*(end-start); juce::Path pointer; pointer.addRoundedRectangle(-1.7f,-r*0.34f,3.4f,r*0.21f,1.5f);
    g.setColour(juce::Colour(0xffffa45e)); g.fillPath(pointer,juce::AffineTransform::rotation(a).translated(cx,cy));
    g.setColour(juce::Colour(0xffc2beb4)); g.drawEllipse(bounds,1.0f);
}

Acid303AudioProcessorEditor::Acid303AudioProcessorEditor(Acid303AudioProcessor& p) : AudioProcessorEditor(p), proc(p) {
    setSize(780, 500);
    setLookAndFeel(&panelLook);
    title.setText("SD-303", juce::dontSendNotification); title.setJustificationType(juce::Justification::centred); title.setFont(juce::Font(30.f, juce::Font::bold)); addAndMakeVisible(title);
    stepLabel.setJustificationType(juce::Justification::centredLeft); addAndMakeVisible(stepLabel);
    juce::Slider* sliders[]={&cutoff,&resonance,&env,&decay,&accent,&wave,&tempo};
    const char* names[]={"CUTOFF","RESONANCE","ENV MOD","DECAY","ACCENT","WAVE","TEMPO"};
    const char* ids[]={"cutoff","resonance","env","decay","accent","wave","tempo"};
    std::unique_ptr<Attachment>* at[]={&cutoffA,&resA,&envA,&decayA,&accentA,&waveA,&tempoA};
    for(int i=0;i<7;++i){ auto* s=sliders[i]; s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,18); s->setLookAndFeel(&panelLook); s->setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xfff2ede0)); s->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff393a3c)); s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff696a69)); s->setName(names[i]); addAndMakeVisible(*s); *at[i]=std::make_unique<Attachment>(proc.state,ids[i],*s); }
    addAndMakeVisible(sequencer); addAndMakeVisible(playing); addAndMakeVisible(stepAccent); addAndMakeVisible(slide);
    seqA=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.state,"sequencer",sequencer);
    playingA=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.state,"playing",playing);
    for(auto* b : {&sequencer,&playing,&stepAccent,&slide}) { b->setColour(juce::ToggleButton::textColourId,juce::Colour(0xff28292a)); b->setColour(juce::ToggleButton::tickColourId,juce::Colour(0xffdb682e)); }
    note.setRange(0,24,1); note.setSliderStyle(juce::Slider::IncDecButtons); note.setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,22); addAndMakeVisible(note);
    for(int i=0;i<16;++i){ steps[i].setButtonText(juce::String(i+1)); steps[i].setColour(juce::TextButton::textColourOffId,juce::Colour(0xff28292a)); steps[i].setColour(juce::TextButton::textColourOnId,juce::Colour(0xff28292a)); steps[i].onClick=[this,i]{ selectedStep=i; updateStepControls(); }; addAndMakeVisible(steps[i]); }
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
void Acid303AudioProcessorEditor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff292a2c)); auto body=getLocalBounds().toFloat().reduced(8);
    g.setColour(juce::Colour(0xffaaa99f)); g.fillRoundedRectangle(body,18); g.setColour(juce::Colour(0xffd7d3c7)); g.fillRoundedRectangle(body.reduced(5),15);
    g.setColour(juce::Colour(0xffb9b5a9)); g.drawRoundedRectangle(body.reduced(5),15,1.2f);
    g.setColour(juce::Colour(0xff393a3a)); g.setFont(juce::Font(26.f,juce::Font::bold)); g.drawText("SD-303",24,16,180,34,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff55534c)); g.setFont(juce::Font(10.f,juce::Font::bold)); g.drawText("ANALOGUE BASS LINE SYNTHESIZER",210,22,330,24,juce::Justification::centred);
    g.setColour(juce::Colour(0xffe46e2e)); g.fillRoundedRectangle(548,19,205,27,4); g.setColour(juce::Colours::white); g.setFont(juce::Font(12.f,juce::Font::bold)); g.drawText("S D  /  ACID SEQUENCER",548,19,205,27,juce::Justification::centred);
    g.setColour(juce::Colour(0xff77756d)); g.drawRoundedRectangle(20,55,740,143,8,1.5f); g.drawRoundedRectangle(20,213,740,229,8,1.5f);
    g.setColour(juce::Colour(0xff5d5b55)); g.setFont(juce::Font(11.f,juce::Font::bold)); g.drawText("PATTERN / TRANSPORT",30,56,220,18,juce::Justification::centredLeft); g.drawText("SOUND CONTROLS",30,215,200,18,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff6c6b65)); g.setFont(juce::Font(10.f,juce::Font::bold)); const char* labels[]={"CUTOFF","RESONANCE","ENV MOD","DECAY","ACCENT","WAVE"};
    for(int i=0;i<6;++i) g.drawText(labels[i],22+i*120,386,112,18,juce::Justification::centred);
    g.drawText("TEMPO",630,169,112,18,juce::Justification::centred);
}
void Acid303AudioProcessorEditor::resized(){
    title.setBounds(0,0,1,1); sequencer.setBounds(35,78,145,24); playing.setBounds(190,78,145,24); tempo.setBounds(630,65,112,112);
    for(int i=0;i<16;++i) steps[i].setBounds(34+(i%8)*88,111+(i/8)*39,74,30);
    stepLabel.setBounds(35,190,285,24); note.setBounds(320,178,115,43); stepAccent.setBounds(455,190,100,24); slide.setBounds(565,190,160,24);
    juce::Slider* s[]={&cutoff,&resonance,&env,&decay,&accent,&wave}; for(int i=0;i<6;++i)s[i]->setBounds(22+i*120,247,112,132);
}
