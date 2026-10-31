#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class Acid303AudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit Acid303AudioProcessorEditor(Acid303AudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    Acid303AudioProcessor& processor;
    juce::Slider cutoff, resonance, env, decay, accent, wave;
    juce::ToggleButton sequencer {"HOST SYNC"};
    juce::Label title;
    using Attachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> cutoffA,resA,envA,decayA,accentA,waveA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> seqA;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Acid303AudioProcessorEditor)
};
