#pragma once
#include <JuceHeader.h>

class Acid303AudioProcessor : public juce::AudioProcessor {
public:
    Acid303AudioProcessor();
    void prepareToPlay(double, int) override;
    void releaseResources() override { playingStep.store(-1); }
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "SD-303"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState state;
    void requestPatternRestart() { restartRequested.store(true); }
    int getPlayingStep() const { return playingStep.load(std::memory_order_relaxed); }
    void auditionNote(int midiNote) { auditionRequested.store(midiNote); }
private:
    struct Voice {
        double phase=0, pitch=36;
        float env=0, amp=0, ic1=0, ic2=0, pole=0, emphasis=0;
        int note=36;
        bool gate=false, glide=false;
    } voice;
    std::array<std::atomic<float>*,16> notes{}, accents{}, slides{};
    std::atomic<float> *cutoffParam{}, *resParam{}, *envParam{}, *decayParam{}, *accentParam{}, *waveParam{}, *volumeParam{};
    juce::SmoothedValue<float> cutoffSmooth, resSmooth, envSmooth, decaySmooth, accentSmooth, volumeSmooth;
    double sampleRate=44100.0, beatPosition=0.0;
    int currentStep=-1, auditionSamples=0;
    bool wasPlaying=false, midiHeld=false;
    std::atomic<bool> restartRequested { false };
    std::atomic<int> playingStep { -1 }, auditionRequested { -1 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Acid303AudioProcessor)
};
