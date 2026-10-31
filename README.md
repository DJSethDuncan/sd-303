<img width="949" height="701" alt="image" src="https://github.com/user-attachments/assets/5c407d4e-769b-425f-89e0-af0db08633f9" />

**Developer:** Seth Duncan  
**Plugin:** SD-303

A JUCE-based, monophonic acid synth VST3 with a host-tempo-synced 16-step pattern and automatable sound controls.

## Build (macOS)

Requires Xcode Command Line Tools, CMake 3.22+, Git, and internet access for JUCE download.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The VST3 bundle is produced under `build/Acid303_artefacts/Release/VST3/SD-303.vst3`. Install it to `~/Library/Audio/Plug-Ins/VST3/` and rescan plugins in your DAW. See [BUILD.md](BUILD.md) for details.

## Features

- Monophonic saw/square oscillator and resonant low-pass filter
- MIDI note input
- Editable 16-step pattern with per-step note/rest, accent, and slide/tie controls
- Host tempo/transport following, plus internal tempo mode
- Automatable cutoff, resonance, envelope modulation, decay, accent, waveform, volume, tempo, and step parameters
- Readouts in Hz, milliseconds, percent, and whole BPM; two-position SAW/SQUARE selection
- Piano-key pitch entry with stopped-mode audition, a running step light, and note names on steps

## Quick start

Open `build/Acid303_artefacts/Release/Standalone/SD-303.app`, select an audio output, then press **RUN / STOP**. New instances start stopped. Click a step and a piano key to change pitch; **REST** clears the note. Gold means selected for editing; red means currently playing. **SLIDE INTO** glides from the previous pitched step without retriggering; **ACCENT** emphasizes that step.

For a pure cutoff sweep, turn **ENV MOD** to zero. Cutoff spans 30 Hz–18 kHz and resonance spans a gentle response to a pronounced peak. Double-click knobs to reset them. See [UX-REVIEW.md](UX-REVIEW.md) for the full review and [BUILD.md](BUILD.md) for offline test commands.

This update changes filter ranges, envelope behavior, and synthesis; older saved settings may sound different. Existing saved sessions can recall RUN enabled even though new instances default to stopped.

This is an initial acid-synth prototype, not an exact TB-303 emulation. Roland and TB-303 are trademarks of their respective owners; this project is unaffiliated.
