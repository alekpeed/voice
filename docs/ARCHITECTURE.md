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

## ADR-009: Notation layout is deterministic and renderer-independent

Notation consumes timestamped pitch, voice, fingering, duration, and chord-symbol
data rather than UI objects. A layout pass resolves staff assignment, pitch
spelling, accidental memory, ledger lines, collision offsets, system wrapping,
scaling, and cursor position before the SVG adapter draws any glyphs. This keeps
engraving behavior testable and permits a later desktop renderer to reuse the
same geometry and document contract.

## ADR-010: Feedback is structured fact, not generated judgment

Feedback is derived from assigned voice transitions and explicit chord context.
The engine reports exact pitches, direction, interval size, common tones,
crossing pairs, and guide-tone arrivals or departures in stable voice order. It
does not infer harmonic function, stylistic quality, or student intent. Later
curriculum and explanation layers may select or explain these observations but
cannot alter the underlying facts.

## ADR-011: The first course path composes existing deterministic subsystems

The fundamental ii-V-I course is a fixed, book-linked sequence rather than a
general exercise generator. Each accepted performance passes through two-voice
assignment, voice-path construction, deterministic feedback, and notation
document creation. Course progression advances only after the expected pitches
are present at every event. This proves the end-to-end learning flow while
leaving transposition and generated solution spaces to Phase 8.

## ADR-012: Generated exercises store constraints and an optimal benchmark

Exercise seeds use a repository-owned stable random sequence rather than
implementation-dependent distribution helpers. The nearest-voicing solver
enumerates the bounded destination register, enforces chord membership, required
tones, voice count, spacing, movement, and optional bass/soprano locks, then
orders valid solutions by total displacement, largest movement, span, and pitch.
Submission re-evaluates the played voicing against those facts, allowing any
equally optimal solution rather than requiring byte-for-byte target imitation.

## ADR-013: Book identity and competency evidence are separate from pagination

Volume I `VL-*` concept IDs are the progress and prerequisite keys. PDF source
IDs, pages, Practice Companion units, and printed exercise IDs are routing
metadata and may change without invalidating progress. Competency is derived
from recorded attempts, hint use, and transposed unassisted success; it is never
assigned directly by a presentation layer.
