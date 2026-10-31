# Acid 303 — Build and Install

Acid 303 is a JUCE-based VST3 synth project. CMake downloads JUCE during configuration, so you need an internet connection for the first build.

## Requirements

- macOS with Xcode Command Line Tools (`xcode-select --install`)
- CMake 3.22 or newer
- Git

Check that the tools are available:

```bash
cmake --version
git --version
```

## Build

Run these commands from the project folder (the folder containing `CMakeLists.txt`):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The VST3 bundle should be created at:

```text
build/Acid303_artefacts/Release/VST3/Acid 303.vst3
```

CMake also defines a Standalone build target.

## Install the VST3

Copy the plugin bundle into your user's VST3 directory:

```bash
mkdir -p "$HOME/Library/Audio/Plug-Ins/VST3"
cp -R "build/Acid303_artefacts/Release/VST3/Acid 303.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/"
```

Then rescan plugins in your DAW, or quit and reopen it. In Ableton Live, use **Preferences → Plug-Ins** to enable VST3 support and rescan if needed.

To remove it, delete `~/Library/Audio/Plug-Ins/VST3/Acid 303.vst3`.

## Notes

- The synth exposes automatable cutoff, resonance, envelope modulation, decay, accent, waveform, and host-sync parameters.
- The host-synced pattern follows host PPQ/tempo while transport is playing. MIDI notes can also be sent to the instrument.
- The pattern and synthesis are an initial implementation, not a complete or exact TB-303 emulation.
- If CMake cannot fetch JUCE, check network access and try configuring again. If compilation fails, verify the Xcode Command Line Tools and CMake versions before rebuilding.
