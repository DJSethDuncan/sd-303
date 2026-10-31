#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class SD303LookAndFeel : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
};
class Acid303AudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit Acid303AudioProcessorEditor(Acid303AudioProcessor&);
    ~Acid303AudioProcessorEditor() override { stopTimer(); setLookAndFeel(nullptr); }
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    Acid303AudioProcessor& proc;
    SD303LookAndFeel panelLook;
    juce::Slider cutoff, resonance, env, decay, accent, wave, tempo;
    juce::ToggleButton sequencer {"HOST TEMPO"}, stepAccent {"ACCENT"}, slide {"SLIDE / TIE"};
    juce::TextButton playing {"RUN / STOP"};
    juce::TextButton restart {"RESET"};
    juce::TextButton steps[16];
    juce::TextButton pianoKeys[24];
    juce::TextButton rest {"REST"};
    juce::Label title, stepLabel;
    using Attachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> cutoffA,resA,envA,decayA,accentA,waveA,tempoA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> seqA,playingA;
    int selectedStep=0;
    int displayedPlayingStep=-1;
    void timerCallback() override;
    void updateStepColours();
    void updateStepControls();
    void setStepNote(int value);
    void updateKeyboard();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Acid303AudioProcessorEditor)
};
