# Architecture

## Required subsystem boundaries
1. MIDI Layer: device enumeration, timestamps, note-on/off, pedal, velocity, latency calibration.
2. Performance State: held notes, sustained notes, attack groups, sonority windows, phrase events.
3. Voice Assignment: continuity, common tones, register, locked soprano/bass, crossing penalties, ambiguity confidence.
4. Harmonic Analysis: intervals, chord candidates, inversion, guide/tendency tones, contextual function.
5. Voice-Leading Evaluation: common tones, semitone/step/leap movement, contrary/similar/parallel/oblique motion, crossing/overlap, guide-tone resolution.
6. Exercise Engine: prompt, constraints, accepted solution space, transposition, deterministic evaluation.
7. Curriculum Engine: concept IDs, prerequisites, book references, competency.
8. Audio Engine: MIDI -> instrument -> FX -> output. Independent of analysis.
9. Visualization: staff, keyboard, voice graph.
10. Persistence: SQLite for settings, presets, attempts, competency, saved labs.
11. Optional AI Tutor: consumes structured deterministic analysis only.

## Threading
Never block audio. Separate real-time audio, MIDI ingestion, analysis worker, UI, persistence, and optional network/AI workers. Never call database or network from audio callback.

## Core objects
Pitch, NoteEvent, Sonority, Voice, VoicePath, VoiceTransition, ChordCandidate, MotionType, ExerciseConstraint, AttemptResult.

## Instrument interface
loadPreset, noteOn, noteOff, pedal, setParameter, renderAudio, getLatency, savePreset. Must support sampled, modeled, hybrid, and later plugin-backed instruments without changing education logic.
