# SD-303 — Build, Run, and Install

## Requirements (macOS)

- Xcode or Command Line Tools (`xcode-select --install`), with the license accepted
- CMake 3.22+ and Git
- Internet access for the initial JUCE download

## Build

Run from the `sd-303` project directory:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release -j4
```

The macOS default is a universal binary (Apple Silicon and Intel). Explicitly specifying the architectures also updates an older Intel-only CMake cache. An Intel-only plugin cannot load in native Apple Silicon Ableton, even if built successfully under Rosetta. Verify with:

```sh
lipo -archs "build/Acid303_artefacts/Release/VST3/SD-303.vst3/Contents/MacOS/SD-303"
```

Both `arm64` and `x86_64` should be listed.

## Run without a DAW

```sh
open "build/Acid303_artefacts/Release/Standalone/SD-303.app"
```

Choose an output device in the Standalone application's audio settings. New instances start stopped: press **RUN / STOP** to play the pattern. Set **VOLUME** above zero. Click a step and then a piano key to program it; the key also briefly auditions while stopped. **REST** silences a step. **RESTART** returns the internal pattern to step 1.

**HOST TEMPO** follows PPQ/tempo when the DAW transport is playing. Otherwise, the instrument uses its own whole-BPM tempo. The RUN button arms pattern playback; MIDI input also works while the pattern is stopped. Saved sessions may recall a running state.

Quit and reopen the app after rebuilding; an already-running instance still uses the previous binary.

## Install the VST3

```sh
mkdir -p "$HOME/Library/Audio/Plug-Ins/VST3"
cp -R "build/Acid303_artefacts/Release/VST3/SD-303.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/"
```

Rescan plugins in your DAW. Ableton lists **SD-303** under **Seth Duncan**. Remove the old `Acid 303.vst3` manually if you installed an earlier build; it is a different plugin identity. These development builds are not notarized.

## Offline regression tests

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSD303_BUILD_TESTS=ON
cmake --build build --config Release -j4
ctest --test-dir build --output-on-failure
```

The test checks startup silence, run/stop/restart, internal and mock-host clocks, MIDI and piano audition, saved patterns, value formatting, filter response, and finite/bounded output during extreme sweeps at 22.05/44.1/48/96 kHz. It also renders a panel snapshot to the test app's cache directory (path printed when running `build/SD303Tests_artefacts/Release/SD303Tests` directly).

The sound engine is an approximate acid-synth model, not a hardware-calibrated TB-303 emulation. See [UX-REVIEW.md](UX-REVIEW.md) for design decisions and limitations.
