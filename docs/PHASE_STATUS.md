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

## Phase 3: Sonority and voice assignment

Implemented:

- Configurable chord windows for rolled chords and small attack offsets.
- Separate held-key, pedal-sustained, and sounding-note state by MIDI channel.
- Deterministic sonority completion for sustain, legato transitions, and repeated notes.
- Bounded sonority history for live sessions.
- Exhaustive weighted assignment for two through four voices.
- Pitch-distance, common-tone, crossing, excess-leap, bass-lock, and soprano-lock costs.
- Confidence and ambiguity derived from the best and second-best mappings.
- Total and maximum displacement, stationary/semitone/whole-step/leap counts,
  crossing, overlap, spacing, bass contour, and soprano contour metrics.
- Exact constraint findings for mismatched voice count, excessive leaps, and failed locks.

Known limitations:

- Voice assignment currently requires equal source and destination voice counts
  from two through four; voice entry and exit are deferred.
- The application timer/UI has not yet been wired to advance the detector; the
  deterministic core exposes explicit time advancement for the later adapter.
- Harmonic labels, guide-tone interpretation, nearest-voicing search, and
  persistent visualization are later phases.

Phase 3 exit evidence: canonical two-, three-, and four-voice assignments,
rolled-chord, sustain, legato, repeated-note, ambiguity, crossing, overlap, and
constraint fixtures pass under the strict build.

## Phase 4: Visualizer

Implemented:

- Persistent voice IDs and labeled paths across multi-sonority progressions.
- Cursor-derived highlighted pitches and a non-decorative relevant keyboard.
- Event timeline markers, viewport zoom, playback cursor, and bounded rates from 0.1x to 1.0x.
- Voice isolation in graph emphasis, keyboard state, and playback events.
- Slow-playback note plans routed through the existing instrument-compatible event types.
- A dark, labeled SVG voice graph with pitch/time axes, event overlays, movement
  labels, stationary/semitone/step/leap descriptions, and per-voice text labels.
- A deterministic four-voice ii-V-I fixture and CLI SVG export.
- Dedicated Visualizer workspace regions in the app shell.

Known limitations:

- The current renderer is the framework-independent SVG adapter; interactive
  desktop widgets and pointer controls await the production UI framework.
- Playback plans are generated but are not yet scheduled into the live sample
  instrument by a visualizer UI controller.
- Chord symbols are fixture labels only; harmonic recognition is not implemented.

Phase 4 exit evidence: the ii-V-I fixture produces four persistent, individually
labeled paths; semitone and stationary guide-tone motion is explicit; isolation,
keyboard state, timeline, zoom, playback rate, SVG export, and the visualizer
smoke command pass under the strict build.

## Phase 5: Notation

Implemented:

- A renderer-independent notation document and deterministic engraving layout.
- Compact grand-staff systems with treble and bass clefs, brace, meter, barlines,
  shared chord stems, filled and open noteheads, and no stretched short phrases.
- Sharp and flat pitch spelling, per-system accidental memory, natural signs,
  ledger lines, adjacent-second notehead offsets, and multi-column accidental
  collision avoidance.
- Chord symbols, persistent voice labels, selectable voice emphasis, optional
  fingering, and a time-interpolated playback cursor.
- Scale control from 0.5x to 2.0x, bounded system width, and deterministic wrapping.
- A four-voice ii-V-I fixture, CLI SVG export, notation smoke command, and study
  workspace controls in the app shell.

Known limitations:

- The framework-independent SVG renderer establishes the notation contract;
  interactive desktop selection and pointer controls remain UI-adapter work.
- Pitch spelling currently uses a global sharp/flat preference and does not infer
  enharmonic spelling from harmonic context or key signatures.
- Rhythmic engraving supports chord-level filled/open noteheads and stems, not
  beams, rests, ties, tuplets, or arbitrary polyphonic rhythm.

Phase 5 exit evidence: compact ii-V-I notation, staff assignment, accidental
state, ledger lines, collision offsets, wrapping, scaling, display toggles,
fingering, voice emphasis, cursor interpolation, SVG export, and the notation
smoke command pass under the strict build and rendered screenshot QA.
