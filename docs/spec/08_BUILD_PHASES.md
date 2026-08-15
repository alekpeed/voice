# Build Phases

## Phase 0 Repository foundation
Repo, build system, CI, tests, app shell, logging, settings, subsystem interfaces. Exit: clean Linux build, tests run, app launches.

## Phase 1 MIDI
Enumerate/connect devices; note/pedal capture; timestamps; monitor; virtual-MIDI test harness. Exit: deterministic fixtures, no stuck notes.

## Phase 2 Audio foundation
Low-latency audio, instrument abstraction, first serious acoustic piano, velocity calibration, sustain, presets. Exit: musically playable extended sessions.

## Phase 3 Sonority + voice assignment
Chord windows, held-note model, assignment, metrics, ambiguity. Exit: canonical 2-4 voice tests pass.

## Phase 4 Visualizer
Voice graph, live keyboard, event timeline, voice isolation, slow playback. Exit: ii-V-I voice paths obvious.

## Phase 5 Notation
Grand staff, compact engraving, chord symbols, voice highlights, scaling, playback cursor. Exit: no stretched notation; screenshot QA approved.

## Phase 6 Deterministic feedback
Common tones, step/leap, crossing, guide-tone movement, contextual feedback. Exit: exact factual feedback.

## Phase 7 Fundamental curriculum
Two-voice motion through ii-V-I. Exit: first end-to-end course path.

## Phase 8 Exercise generator
All-key generation, nearest voicing, fixed soprano/bass, guide tones, 2/3/4 voices, deterministic seeds.

## Phase 9 Book integration
Stable VL IDs, book references, prerequisites, competency tracking.

## Phase 10 Instrument expansion
Upright, felt, Rhodes, Wurlitzer, Workshop, EQ, tuning, FX, presets. Exit: EPs credible for long practice sessions.

## Phase 11 Melody harmonization
Fixed melody, candidate solutions, rankings, voice-path comparison.

## Phase 12 Inner-line / reharmonization lab
Voice locks, user-defined lines, independent bass, alternative harmony, saved labs.

## Phase 13 Barry Harris foundation
Major/minor sixth-diminished, inversions, family classification, melody harmonization, all keys.

## Phase 14 Barry Harris advanced
Borrowing, related dominants, minor-6/dominant relationship, passing diminished, free lab.

## Phase 15 Ear training
Moving-voice identification, direction, common tones, voice isolation, reconstruction.

## Phase 16 Free-play analyzer
Record MIDI, segment session, browse voice paths, detect recurring habits.

## Phase 17 Tune/progression practice
User-entered progressions, comping constraints, chorus-based tasks.

## Phase 18 Optional AI tutor
Structured payload only; AI explains deterministic findings; offline app remains complete without it.

## Phase 19 Polish/release
Performance, onboarding, device robustness, import/export, backups, accessibility, packaging, documentation.

Dependency chain: MIDI -> performance state -> voice assignment -> analysis -> visualizer -> exercises -> curriculum -> Barry Harris -> free analysis -> AI.
