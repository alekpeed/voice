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
