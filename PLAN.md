# SD-303 redesign plan

## Goal
Turn the current prototype into a playable, editable TB-303-inspired instrument with a recognizable control workflow and a sound engine that can be iteratively tuned against references. This is an independent emulation, not a claim of exact hardware equivalence.

## Work sequence
1. **Replace the fixed phrase with a real pattern model.** Store 16 steps with note/rest, octave, accent, and slide/tie; persist all steps in the plugin state. Provide editable step controls and a pattern reset.
2. **Implement 303-style sequencing.** Advance steps from host PPQ/tempo when host sync is enabled, with a usable internal tempo clock otherwise. Trigger envelope and glide consistently at step boundaries; distinguish rests, ties, and slides.
3. **Build the instrument panel around the original workflow.** Add pitch/time mode, pattern playback/write mode controls, 16 step selectors, and classic synth controls (tuning, cutoff, resonance, envelope modulation, decay, accent, waveform). All continuous controls and toggles should be host-automatable.
4. **Improve the synthesis model.** Replace the one-pole filter with a stable resonant nonlinear low-pass model; implement oscillator waveform, per-note envelope, accent emphasis, and portamento on slide steps. Keep DSP real-time safe.
5. **Build, inspect, test, and tune.** Build both VST3 and standalone; verify state save/restore, parameter exposure, transport behavior, and that patterns are editable. Tune sound and control ranges by listening against reference recordings/hardware where available.

## Execution status
- Step 1: implemented 16 independently editable note/rest steps, with per-step accent and slide/tie flags persisted as host parameters/state.
- Step 2: implemented host PPQ/tempo clocking plus an internal-tempo run mode.
- Step 3: added 16-step selector, selected-step note/rest editor, accent/slide toggles, run/host-sync toggles, and synth controls. The original hardware's full pattern bank/mode workflow is not yet implemented.
- Step 4: partial first pass. Accent affects output level; slide currently holds/glides state in a basic manner, and the filter remains an approximate feedback low-pass, not a validated diode-ladder model.
- Step 5: VST3 and Standalone build successfully. Pattern editing/state and transport behavior still need in-host functional testing, and perceptual similarity needs reference-based listening/tuning.
