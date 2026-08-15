# Phase Status

## Phase 0: Repository foundation

Implemented:

- C++20 build with CMake and a dependency-free Make fallback.
- Linux CI configuration and smoke test.
- Domain value types and all required subsystem contracts.
- App-shell navigation and three-column workspace model.
- Settings validation and file persistence.
- Thread-safe structured logger.
- Minimal deterministic test harness and initial tests.

Deliberately deferred:

- JUCE desktop widgets and device adapters.
- SQLite adapter.
- MIDI, audio, music analysis, notation, visualization, exercises, curriculum, and Barry Harris implementations.

Phase 0 exit evidence: build succeeds, tests pass, and `voice-leading-lab --smoke-test` exits successfully.

## Phase 1: MIDI

Implemented:

- Stateful MIDI 1.0 byte-stream parser with running status and real-time-byte tolerance.
- Note-on, note-off, velocity-zero note-off, channel, sustain-pedal, and monotonic timestamp handling.
- Linux raw-MIDI device enumeration and nonblocking input worker.
- Virtual MIDI device and deterministic fixture playback.
- Bounded event monitoring.
- Duplicate-attack recovery and synthetic note/pedal release on disconnect to prevent stuck notes.

Known limitation:

- Linux hardware enumeration currently targets `/dev/snd/midiC*D*` raw-MIDI devices. A later JUCE adapter will add cross-platform CoreMIDI, Windows MIDI, and ALSA-sequencer discovery behind the same interface.

Phase 1 exit evidence: parser/session fixtures pass and `voice-leading-lab --midi-smoke` exits successfully.

## Phase 2: Audio foundation

Implemented:

- Lock-free MIDI-to-audio command queue and polyphonic sample-piano renderer.
- Sample-rate conversion by linear interpolation, velocity layers, pitch mapping, stereo pan/gain, attack smoothing, release envelopes, voice stealing, sustain, and release-trigger regions.
- SFZ subset loader with global/group inheritance and cached WAV samples.
- PCM 16/24/32-bit and float 32-bit mono/stereo WAV decoding.
- Linear, soft, hard, and device-calibrated custom velocity curves.
- Linux ALSA low-latency output loaded dynamically, so the core still builds without development headers.
- MIDI-to-instrument routing and deterministic audio smoke tests.

Known limitations:

- Piano recordings are intentionally external and require a legally obtained SFZ/WAV sample pack.
- The initial engine preloads samples into memory. Disk streaming, advanced SFZ opcodes, sympathetic resonance, half-pedal layers, una corda, and automatic device-latency measurement remain later audio work.
- Physical audio hardware and a full piano sample pack were unavailable in CI; rendering is validated deterministically in memory.

Phase 2 exit evidence: strict builds, audio unit and dense-render stress tests, sanitizer tests, `voice-leading-lab --audio-smoke`, and ALSA null-device output pass.

## Cross-phase curriculum alignment

The three study PDFs uploaded to the repository are now authoritative inputs to
all later phases. A typed catalog indexes the 64 published voice-leading concept
IDs, 16 matching practice units, 80 printed exercises, 12 etudes, 33 Barry
Harris chapters, and 5 appendices. It also preserves edition hashes and current
page routing while keeping stable IDs independent of pagination.

This work establishes content identity and coverage only. Exercise execution,
book navigation, notation, scoring, persistence, and adaptive scheduling remain
in their designated implementation phases.
