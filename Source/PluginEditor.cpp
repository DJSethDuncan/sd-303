#include "PluginEditor.h"

void SD303LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) {
    auto r=juce::jmin(w,h)*0.72f; auto cx=x+w*0.5f, cy=y+h*0.45f; auto bounds=juce::Rectangle<float>(cx-r*0.5f,cy-r*0.5f,r,r);
    for(int i=0;i<=10;++i){
        auto a=start+(end-start)*(float)i/10.f;
        g.setColour(juce::Colour(0xff535d59));
        g.drawLine(cx+std::sin(a)*r*.56f,cy-std::cos(a)*r*.56f,cx+std::sin(a)*r*.64f,cy-std::cos(a)*r*.64f,1.5f);
    }
    g.setColour(juce::Colour(0xff222326)); g.fillEllipse(bounds.translated(0,2));
    g.setColour(juce::Colour(0xff424448)); g.fillEllipse(bounds);
    g.setColour(juce::Colour(0xfff17b32)); g.drawEllipse(bounds.reduced(2),2.0f);
    g.setColour(juce::Colour(0xffd7d1c3)); g.fillEllipse(bounds.reduced(r*0.16f));
    g.setColour(juce::Colour(0xff26272a)); g.fillEllipse(bounds.reduced(r*0.26f));
    auto a=start+pos*(end-start); juce::Path pointer; pointer.addRoundedRectangle(-1.7f,-r*0.34f,3.4f,r*0.21f,1.5f);
    g.setColour(juce::Colour(0xffffa45e)); g.fillPath(pointer,juce::AffineTransform::rotation(a).translated(cx,cy));
    g.setColour(juce::Colour(0xffc2beb4)); g.drawEllipse(bounds,1.0f);
}

void SD303LookAndFeel::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool over,bool down){
    if(!b.getProperties().getWithDefault("pianoKey",false)) { juce::LookAndFeel_V4::drawButtonText(g,b,over,down); return; }
    g.setColour(b.findColour(juce::TextButton::textColourOffId));
    g.setFont(juce::Font(11.f));
    g.drawText(b.getButtonText(),2,b.getHeight()-24,b.getWidth()-4,20,juce::Justification::centred);
}

Acid303AudioProcessorEditor::Acid303AudioProcessorEditor(Acid303AudioProcessor& p) : AudioProcessorEditor(p), proc(p) {
    setSize(960, 710);
    setLookAndFeel(&panelLook);
    title.setText("SD-303", juce::dontSendNotification); title.setJustificationType(juce::Justification::centred); title.setFont(juce::Font(30.f, juce::Font::bold)); addAndMakeVisible(title);
    stepLabel.setJustificationType(juce::Justification::centredLeft); addAndMakeVisible(stepLabel);
    juce::Slider* sliders[]={&cutoff,&resonance,&env,&decay,&accent,&wave,&tempo,&volume};
    const char* names[]={"CUTOFF","RESONANCE","ENV MOD","DECAY","ACCENT","WAVE","TEMPO","VOLUME"};
    const char* ids[]={"cutoff","resonance","env","decay","accent","wave","tempo","volume"};
    const char* tips[]={"Base filter cutoff: 30 Hz to 18000 Hz. Lower ENV MOD for a pure filter sweep.","Resonant peak: 0 to 100%. High settings strongly emphasize the cutoff frequency.","Filter envelope depth. Zero leaves cutoff fixed; full adds up to four octaves (more on accents).", "Envelope decay in milliseconds. Shapes the length of each note and filter sweep.","Accent strength. Affects volume and filter emphasis on accented steps only.","Choose SAW or SQUARE. Two discrete oscillator shapes.","Internal tempo in whole BPM. Used unless HOST TEMPO follows a playing DAW.","Master output level. Zero mutes the instrument."};
    std::unique_ptr<Attachment>* at[]={&cutoffA,&resA,&envA,&decayA,&accentA,&waveA,&tempoA,&volumeA};
    for(int i=0;i<8;++i){ auto* s=sliders[i]; s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,18); s->setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xfff2ede0)); s->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff393a3c)); s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff696a69)); s->setName(names[i]); addAndMakeVisible(*s); *at[i]=std::make_unique<Attachment>(proc.state,ids[i],*s);
        s->setRotaryParameters(juce::MathConstants<float>::pi*7.f/6.f,juce::MathConstants<float>::pi*17.f/6.f,true);
        s->setDoubleClickReturnValue(true,proc.state.getParameter(ids[i])->convertFrom0to1(proc.state.getParameter(ids[i])->getDefaultValue()));
        s->setTooltip(juce::String(tips[i])+" Double-click resets; Shift-drag for fine adjustment.");
    }
    tempo.textFromValueFunction=[](double v){ return juce::String(juce::roundToInt(v))+" BPM"; };
    tempo.updateText();
    addAndMakeVisible(sequencer); addAndMakeVisible(playing); addAndMakeVisible(restart); addAndMakeVisible(stepAccent); addAndMakeVisible(slide);
    restart.setButtonText("RESTART");
    restart.setTooltip("Return to step 1 on the internal clock. In a playing DAW, host position remains authoritative.");
    restart.onClick=[this]{ proc.requestPatternRestart(); };
    rest.setTooltip("Make the selected step silent. Rests release the note even if Slide is enabled.");
    stepAccent.setTooltip("Emphasize this step's volume and filter envelope. Strength is set by the ACCENT knob.");
    slide.setButtonText("SLIDE INTO");
    slide.setTooltip("Glide into this step from the preceding note without retriggering the envelope. Has no effect on a rest.");
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
    for (int k=0;k<24;++k) {
        auto& key=pianoKeys[k];
        key.setButtonText(juce::MidiMessage::getMidiNoteName(36+k,true,true,4));
        key.getProperties().set("pianoKey",true);
        key.setConnectedEdges(juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
        key.setTooltip("Assign "+key.getButtonText()+" (MIDI "+juce::String(36+k)+") to the selected step. Auditions while stopped.");
        key.onClick=[this,k]{ setStepNote(k+1); proc.auditionNote(36+k); };
        addAndMakeVisible(key);
    }
    rest.onClick=[this]{ setStepNote(0); };
    addAndMakeVisible(rest);
    for(int i=0;i<16;++i){ steps[i].setButtonText(juce::String(i+1)); steps[i].setColour(juce::TextButton::textColourOffId,juce::Colour(0xff28292a)); steps[i].setColour(juce::TextButton::textColourOnId,juce::Colour(0xff28292a)); steps[i].onClick=[this,i]{ selectedStep=i; updateStepControls(); }; addAndMakeVisible(steps[i]); }
    stepAccent.onClick=[this]{ auto* param=proc.state.getParameter("accent"+juce::String(selectedStep+1)); if(param) { param->beginChangeGesture(); param->setValueNotifyingHost(stepAccent.getToggleState()?1.f:0.f); param->endChangeGesture(); } };
    slide.onClick=[this]{ auto* param=proc.state.getParameter("slide"+juce::String(selectedStep+1)); if(param) { param->beginChangeGesture(); param->setValueNotifyingHost(slide.getToggleState()?1.f:0.f); param->endChangeGesture(); } };
    updateStepControls();
    resized(); // Establish black-key z-order after all children have been added.
    startTimerHz(30);
}
void Acid303AudioProcessorEditor::timerCallback(){
    displayedPlayingStep=proc.state.getRawParameterValue("playing")->load()>.5f ? proc.getPlayingStep() : -1;
    updateStepControls();
    repaint(34,348,124,24);
}
void Acid303AudioProcessorEditor::updateStepColours(){
    for(int i=0;i<16;++i){
        const bool active=i==displayedPlayingStep;
        const bool selected=i==selectedStep;
        const bool hasNote=proc.state.getRawParameterValue("step"+juce::String(i+1))->load()>0;
        steps[i].setColour(juce::TextButton::buttonColourId,active ? juce::Colour(0xfff04432) : selected ? juce::Colour(0xffd2ac65) : hasNote ? juce::Colour(0xff585d5c) : juce::Colour(0xff363a3c));
        steps[i].setColour(juce::TextButton::textColourOffId,selected && !active ? juce::Colour(0xff202525) : juce::Colours::white);
        const int value=(int)proc.state.getRawParameterValue("step"+juce::String(i+1))->load();
        steps[i].setButtonText(juce::String(i+1)+"  "+(value==0 ? juce::String("REST") : juce::MidiMessage::getMidiNoteName(35+value,true,true,4)));
    }
}
void Acid303AudioProcessorEditor::updateStepControls(){
    auto suffix=juce::String(selectedStep+1);
    stepAccent.setToggleState(proc.state.getRawParameterValue("accent"+suffix)->load()>.5f,juce::dontSendNotification);
    slide.setToggleState(proc.state.getRawParameterValue("slide"+suffix)->load()>.5f,juce::dontSendNotification);
    updateKeyboard();
    updateStepColours();
}
void Acid303AudioProcessorEditor::setStepNote(int value){
    if (auto* param=proc.state.getParameter("step"+juce::String(selectedStep+1))) {
        param->beginChangeGesture();
        param->setValueNotifyingHost(param->convertTo0to1((float)value));
        param->endChangeGesture();
    }
    updateStepControls();
}
void Acid303AudioProcessorEditor::updateKeyboard(){
    const int value=(int)proc.state.getRawParameterValue("step"+juce::String(selectedStep+1))->load();
    stepLabel.setText("STEP "+juce::String(selectedStep+1)+"   /   "+(value==0 ? juce::String("REST") : juce::MidiMessage::getMidiNoteName(35+value,true,true,4)),juce::dontSendNotification);
    for(int k=0;k<24;++k){
        const int pc=k%12; const bool black=pc==1 || pc==3 || pc==6 || pc==8 || pc==10;
        pianoKeys[k].setColour(juce::TextButton::buttonColourId,value==k+1 ? juce::Colour(0xffd7a653) : black ? juce::Colour(0xff202628) : juce::Colour(0xffedece3));
        pianoKeys[k].setColour(juce::TextButton::textColourOffId,black && value!=k+1 ? juce::Colours::white : juce::Colour(0xff202525));
    }
    rest.setColour(juce::TextButton::buttonColourId,value==0 ? juce::Colour(0xffd7a653) : juce::Colour(0xff42494a));
}
void Acid303AudioProcessorEditor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff25282a));
    juce::ColourGradient metal(juce::Colour(0xffe0e2dc),0,10,juce::Colour(0xffa6afab),0,700,false);
    g.setGradientFill(metal); g.fillRoundedRectangle(8,8,944,694,12);
    g.setColour(juce::Colour(0xff777f7d)); g.drawRoundedRectangle(12,12,936,686,10,1);
    g.setColour(juce::Colour(0xff242b2b)); g.setFont(juce::Font(27.f,juce::Font::bold));
    g.drawText("Seth Duncan",28,30,210,40,juce::Justification::centredLeft);
    g.setFont(juce::Font(13.f,juce::Font::bold)); g.drawText("BASS LINE",28,78,210,24,juce::Justification::centredLeft);
    g.drawLine(248,24,248,194,1); g.drawLine(20,202,940,202,2);
    const char* labels[]={"WAVEFORM","CUT OFF FREQ","RESONANCE","ENV MOD","DECAY","ACCENT"};
    g.setFont(juce::Font(11.f,juce::Font::bold));
    for(int i=0;i<6;++i) g.drawText(labels[i],262+i*112,26,104,20,juce::Justification::centred);
    g.drawText("TEMPO",28,210,140,20,juce::Justification::centred);
    g.drawText("VOLUME",505,210,120,20,juce::Justification::centred);
    g.setFont(juce::Font(32.f,juce::Font::bold)); g.drawText("SD-303",660,229,240,42,juce::Justification::centredRight);
    g.setFont(juce::Font(14.f)); g.drawText("Computer Controlled",660,272,240,22,juce::Justification::centredRight);
    g.setColour(juce::Colour(0xff343c3b)); g.drawLine(20,322,940,322,2);
    g.setColour(juce::Colour(0xff222728)); g.fillRoundedRectangle(184,340,748,126,5);
    g.setColour(juce::Colour(0xffedefe6)); g.setFont(juce::Font(11.f,juce::Font::bold)); g.drawText("PATTERN STEPS",200,346,250,18,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff303635)); g.drawRect(26,340,140,190,1);
    const bool running=displayedPlayingStep>=0;
    g.setColour(running ? juce::Colour(0xffff3828) : juce::Colour(0xff58332f));
    g.fillEllipse(46,354,12,12);
    g.setColour(juce::Colour(0xff303635)); g.setFont(juce::Font(11.f,juce::Font::bold));
    g.drawText(running ? "RUNNING" : "STOPPED",65,348,88,24,juce::Justification::centredLeft);
    g.setFont(juce::Font(12.f,juce::Font::bold)); g.drawText("PATTERN",34,448,124,22,juce::Justification::centred);
    g.setColour(juce::Colour(0xff303635)); g.setFont(juce::Font(11.f,juce::Font::bold));
    g.drawText("PITCH ENTRY  /  SELECT A STEP ABOVE, THEN CLICK A KEY",198,520,710,22,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff222728)); g.fillRoundedRectangle(190,546,506,138,4);
    g.setColour(juce::Colour(0xff303635)); g.setFont(juce::Font(12.f));
    g.drawText("GOLD: selected for editing",714,558,220,24,juce::Justification::centredLeft);
    g.drawText("RED: currently playing",714,585,220,24,juce::Justification::centredLeft);
    g.drawText("Click a key to set pitch.",714,622,220,24,juce::Justification::centredLeft);
    g.drawText("Audition available when stopped.",714,646,220,24,juce::Justification::centredLeft);
    for(auto point : {juce::Point<int>(20,20),juce::Point<int>(940,20),juce::Point<int>(20,690),juce::Point<int>(940,690)}) {
        g.setColour(juce::Colour(0xff737b78)); g.fillEllipse((float)point.x-3,(float)point.y-3,6,6);
        g.setColour(juce::Colour(0xff424a47)); g.drawLine((float)point.x-2,(float)point.y,(float)point.x+2,(float)point.y,1);
    }
}
void Acid303AudioProcessorEditor::resized(){
    title.setVisible(false);
    juce::Slider* controls[]={&wave,&cutoff,&resonance,&env,&decay,&accent};
    for(int i=0;i<6;++i) controls[i]->setBounds(262+i*112,52,104,130);
    tempo.setBounds(28,230,140,84);
    volume.setBounds(505,230,120,84);
    sequencer.setBounds(198,245,150,28);
    restart.setBounds(376,242,105,34);
    playing.setBounds(42,374,108,66);
    for(int i=0;i<16;++i) steps[i].setBounds(200+(i%8)*90,373+(i/8)*44,78,34);
    stepLabel.setBounds(198,478,300,22);
    rest.setBounds(512,478,100,34);
    int whiteIndex=0;
    for(int k=0;k<24;++k){
        const int pc=k%12; const bool black=pc==1 || pc==3 || pc==6 || pc==8 || pc==10;
        if(!black) { pianoKeys[k].setBounds(198+whiteIndex*35,554,35,122); ++whiteIndex; }
        else pianoKeys[k].setBounds(198+whiteIndex*35-36/2,554,36,74);
    }
    for(int k=0;k<24;++k){ const int pc=k%12; if(pc==1 || pc==3 || pc==6 || pc==8 || pc==10) pianoKeys[k].toFront(false); }
    stepAccent.setBounds(648,484,110,28);
    slide.setBounds(775,484,145,28);
}
