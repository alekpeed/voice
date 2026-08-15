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
