# SD-303 usability and sound review

## Findings and decisions

| Component | Problem found | Decision |
|---|---|---|
| Startup | Automatic playback is surprising; stopped state is not an error. | New instances start stopped. RUN starts at step 1; STOP releases the pattern voice. Saved host automation/state still recalls the Run parameter. |
| Transport | STOP left a note sounding indefinitely; internal clock and host clock behavior were ambiguous. | Separate pattern gate from MIDI gate, release on stop, restart reliably, and explain host-tempo fallback. |
| Cutoff | The envelope added up to 8 kHz even at minimum cutoff, obscuring most of the knob's range. One-pole filter barely removed harmonics. | 30 Hz–18 kHz base cutoff with a perceptual midpoint; envelope modulation in octaves instead of additive Hz; resonant 18 dB/oct approximate low-pass. |
| Resonance | Feedback in the old one-pole filter mostly increased volume, not a resonant peak. | Stable topology-preserving state-variable resonator plus a low-pass stage. Q spans 0.5–18; bounded output. This is not a circuit-accurate 303 filter. |
| Knob motion | Low visual contrast, no scale, tiny tempo knob. | 300-degree sweep, scale marks, clear indicator, full-size tempo knob, double-click resets and fine Shift-drag. |
| Readouts | Normalized decimals are implementation details. | Whole Hz, milliseconds, percentages, BPM, and SAW/SQUARE instead of raw floats. Host value text follows these units too. |
| Waveform | A continuously variable blend is not a clear two-wave instrument. | Snap to saw or square; keep the existing parameter ID. |
| Decay | Only filter decay worked; the amplifier stayed open. | Make decay shape both amplifier and filter envelopes, with a fast attack and stop/release tail. |
| Accent | Only loudness changed, with sample-loop parameter lookups. | Step-latched accent boosts level and envelope emphasis. |
| Slide | Merely avoided a retrigger; there was no portamento. | Slide on a step glides INTO that step over about 60 ms, without retrigger. Explicit tooltip documents this convention. |
| Piano | No audition while stopped; note names depended on DAW octave convention. | Click a key to assign AND briefly audition while stopped. Retain C2–B3 labels (MIDI 36–59); MIDI numbering is authoritative. |
| Step feedback | Playback hid selection; no note names on steps. | Show step number plus pitch/rest, separate selected and playing colors, explicit legend. Refresh accent/slide when host automation changes. |
| Rest | Rest could leave the oscillator held or become a fake tie. | A rest always releases. Slide is meaningful only on a pitched step. |
| Host automation | Abrupt filter changes could click; string construction and parameter lookup occurred per audio sample. | Smooth tone controls and cache parameter pointers; snapshot step values once per block. |
| Output | No volume control or level headroom. | Add automatable volume in percent, conservative default, soft output saturation. |
| Layout | Oversized keyboard backdrop, tiny tempo, unlabeled unused space. | Use keyboard spare area for instructions/legend; reserve separate transport and volume columns. No fake controls for unimplemented original modes. |

## Validation
VST3, Standalone, and the offline regression test all built successfully. Tests pass for startup silence, run/stop/restart, internal tempo/step progression, mock host position/loop/fallback, MIDI, preview, state recall, formatting, and extreme sweeps at 22.05/44.1/48/96 kHz. Output remains finite/bounded in these tests. An isolated note render measured RMS 0.00247 at 30 Hz cutoff versus 0.19218 at 12 kHz, and 0.12241 at zero resonance versus 0.47740 at maximum resonance near the note frequency. These numbers demonstrate control effect, not hardware fidelity.

Rendered and inspected a complete editor snapshot. This exposed a black-key stacking bug: adding children after the initial layout put white keys over black keys. Fixed final z-order and added a hit-test regression. Key names are now placed at the bottom of keys in a smaller font so they remain readable.

## Limitations
This is still an approximate acid synth, not a measured TB-303 circuit emulation. No track chains, pattern banks, original pitch/time programming modes, or hardware-calibrated accent/slide model. Listening tests against hardware/reference recordings and actual DAW/Standalone interaction remain necessary; passing offline tests does not establish perceptual fidelity.
