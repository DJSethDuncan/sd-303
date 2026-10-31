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

The VST3 bundle is produced under `build/SD303_artefacts/Release/VST3/` (or the corresponding target artefact directory reported by CMake). Install it to `~/Library/Audio/Plug-Ins/VST3/` and rescan plugins in your DAW. See [BUILD.md](BUILD.md) for details.

## Features

- Monophonic saw/square oscillator and resonant low-pass filter
- MIDI note input
- Editable 16-step pattern with per-step note/rest, accent, and slide/tie controls
- Host tempo/transport following, plus internal tempo mode
- Automatable cutoff, resonance, envelope modulation, decay, accent, waveform, tempo, and step parameters

This is an initial acid-synth prototype, not an exact TB-303 emulation. Roland and TB-303 are trademarks of their respective owners; this project is unaffiliated.
