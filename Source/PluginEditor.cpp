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
    setSize(960, 560);
    setLookAndFeel(&panelLook);
    title.setText("SD-303", juce::dontSendNotification); title.setJustificationType(juce::Justification::centred); title.setFont(juce::Font(30.f, juce::Font::bold)); addAndMakeVisible(title);
    stepLabel.setJustificationType(juce::Justification::centredLeft); addAndMakeVisible(stepLabel);
    juce::Slider* sliders[]={&cutoff,&resonance,&env,&decay,&accent,&wave,&tempo};
    const char* names[]={"CUTOFF","RESONANCE","ENV MOD","DECAY","ACCENT","WAVE","TEMPO"};
    const char* ids[]={"cutoff","resonance","env","decay","accent","wave","tempo"};
    std::unique_ptr<Attachment>* at[]={&cutoffA,&resA,&envA,&decayA,&accentA,&waveA,&tempoA};
    for(int i=0;i<7;++i){ auto* s=sliders[i]; s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,18); s->setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xfff2ede0)); s->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff393a3c)); s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff696a69)); s->setName(names[i]); addAndMakeVisible(*s); *at[i]=std::make_unique<Attachment>(proc.state,ids[i],*s); }
    addAndMakeVisible(sequencer); addAndMakeVisible(playing); addAndMakeVisible(restart); addAndMakeVisible(stepAccent); addAndMakeVisible(slide);
    restart.onClick=[this]{ proc.requestPatternRestart(); };
    playing.setTooltip("Start or stop the built-in pattern. It runs in Standalone without a DAW transport.");
    sequencer.setTooltip("Follow host tempo when running in a DAW; otherwise use the Tempo control.");
    seqA=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.state,"sequencer",sequencer);
    playing.setClickingTogglesState(true);
    playing.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff535657));
    playing.setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xffae3630));
    playing.setColour(juce::TextButton::textColourOffId,juce::Colours::white);
    playing.setColour(juce::TextButton::textColourOnId,juce::Colours::white);
    playingA=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.state,"playing",playing);
    stepLabel.setColour(juce::Label::textColourId,juce::Colour(0xff252728));
    for(auto* b : {&sequencer,&stepAccent,&slide}) { b->setColour(juce::ToggleButton::textColourId,juce::Colour(0xff28292a)); b->setColour(juce::ToggleButton::tickColourId,juce::Colour(0xffdb682e)); }
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
    for(int i=0;i<16;++i){ int v=(int)proc.state.getRawParameterValue("step"+juce::String(i+1))->load(); steps[i].setColour(juce::TextButton::buttonColourId,i==selectedStep?juce::Colour(0xffa73730):(v?juce::Colour(0xff585d5c):juce::Colour(0xff363a3c))); steps[i].setColour(juce::TextButton::textColourOffId,juce::Colours::white); }
}
void Acid303AudioProcessorEditor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff25282a));
    juce::ColourGradient metal(juce::Colour(0xffe0e2dc),0,10,juce::Colour(0xffa6afab),0,550,false);
    g.setGradientFill(metal); g.fillRoundedRectangle(8,8,944,544,12);
    g.setColour(juce::Colour(0xff777f7d)); g.drawRoundedRectangle(12,12,936,536,10,1);
    g.setColour(juce::Colour(0xff242b2b)); g.setFont(juce::Font(27.f,juce::Font::bold));
    g.drawText("Seth Duncan",28,30,210,40,juce::Justification::centredLeft);
    g.setFont(juce::Font(13.f,juce::Font::bold)); g.drawText("BASS LINE",28,78,210,24,juce::Justification::centredLeft);
    g.drawLine(248,24,248,194,1); g.drawLine(20,202,940,202,2);
    const char* labels[]={"WAVEFORM","CUT OFF FREQ","RESONANCE","ENV MOD","DECAY","ACCENT"};
    g.setFont(juce::Font(11.f,juce::Font::bold));
    for(int i=0;i<6;++i) g.drawText(labels[i],262+i*112,26,104,20,juce::Justification::centred);
    g.drawText("TEMPO",28,218,140,20,juce::Justification::centred);
    g.setFont(juce::Font(32.f,juce::Font::bold)); g.drawText("SD-303",660,229,240,42,juce::Justification::centredRight);
    g.setFont(juce::Font(14.f)); g.drawText("Computer Controlled",660,272,240,22,juce::Justification::centredRight);
    g.setColour(juce::Colour(0xff343c3b)); g.drawLine(20,322,940,322,2);
    g.setColour(juce::Colour(0xff222728)); g.fillRoundedRectangle(184,340,748,126,5);
    g.setColour(juce::Colour(0xffedefe6)); g.setFont(juce::Font(11.f,juce::Font::bold)); g.drawText("PATTERN STEPS",200,346,250,18,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff303635)); g.drawRect(26,340,140,190,1);
    g.setFont(juce::Font(12.f,juce::Font::bold)); g.drawText("RUN / STOP",34,448,124,22,juce::Justification::centred);
    for(auto point : {juce::Point<int>(20,20),juce::Point<int>(940,20),juce::Point<int>(20,540),juce::Point<int>(940,540)}) {
        g.setColour(juce::Colour(0xff737b78)); g.fillEllipse((float)point.x-3,(float)point.y-3,6,6);
        g.setColour(juce::Colour(0xff424a47)); g.drawLine((float)point.x-2,(float)point.y,(float)point.x+2,(float)point.y,1);
    }
}
void Acid303AudioProcessorEditor::resized(){
    title.setVisible(false);
    juce::Slider* controls[]={&wave,&cutoff,&resonance,&env,&decay,&accent};
    for(int i=0;i<6;++i) controls[i]->setBounds(262+i*112,52,104,130);
    tempo.setBounds(28,242,140,64);
    sequencer.setBounds(198,245,150,28);
    restart.setBounds(376,242,105,34);
    playing.setBounds(42,374,108,66);
    for(int i=0;i<16;++i) steps[i].setBounds(200+(i%8)*90,373+(i/8)*44,78,34);
    stepLabel.setBounds(198,478,300,22);
    note.setBounds(512,476,110,56);
    stepAccent.setBounds(648,484,110,28);
    slide.setBounds(775,484,145,28);
}
