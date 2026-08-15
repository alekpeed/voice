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
