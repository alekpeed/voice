# Architecture Decisions

## ADR-001: Voice paths are the central domain object

Chord symbols are metadata. Analysis must preserve note events, sonorities, voice assignments, voice transitions, confidence, and explanatory facts.

## ADR-002: Subsystems communicate through narrow contracts

MIDI, performance state, voice assignment, harmonic analysis, evaluation, exercises, curriculum, audio, notation, visualization, persistence, and the application shell are independently replaceable.

## ADR-003: Deterministic core, optional explanation layer

Evaluation results originate in deterministic code. A future tutor may explain structured results but cannot manufacture analysis facts.

## ADR-004: Real-time work is isolated

The future audio callback will not perform database, network, allocation-heavy, or UI work. MIDI timestamps remain monotonic and are carried into analysis.

## ADR-005: External technology is adapted at the boundary

JUCE, SQLite, notation rendering, sampling/modeling, and plugin hosting will enter through adapters. Domain headers contain no framework types.

## ADR-006: Sonorities are time-windowed performance states

A configurable attack window groups rolled or slightly asynchronous notes. The
performance state tracks physical keys separately from pedal-sustained sound,
and legato releases inside the active window affect the destination sonority.
Time advancement is explicit so tests and offline analysis remain deterministic.

## ADR-007: Small voice assignments use exhaustive weighted matching

For two through four voices, all mappings are cheap enough to evaluate. The
deterministic cost model combines pitch distance, common-tone continuity,
crossing, excess leaps, and hard outer-voice locks. The best and second-best
costs produce explicit ambiguity and confidence instead of hiding uncertain
assignments.

## ADR-008: Visualization consumes persistent paths, not chord labels

The visualizer converts transition-local assignments into stable voice IDs over
an event sequence. Its frame contains paths, cursor, viewport, event markers,
relevant keyboard state, isolation, and playback rate. A deterministic SVG
adapter proves the rendering contract without coupling the core to a desktop UI
framework. Slow and isolated playback are emitted as timestamped note events for
the existing instrument boundary.
