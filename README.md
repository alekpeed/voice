# Voice Leading Lab

Desktop-first piano training software centered on voice paths rather than chord labels.

## Current milestone

Phase 0 repository foundation. The project currently provides subsystem contracts, app-shell state, logging, settings, deterministic tests, and CI. MIDI, audio, music analysis, notation, persistence, and curriculum implementations intentionally begin in later phases.

## Build

Preferred CMake build:

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
./build/voice-leading-lab
```

Minimal Linux fallback when CMake is unavailable:

```sh
make test
make run
```

## Architecture

Subsystem boundaries are defined under `include/vll`. The user interface and audio technology remain replaceable behind interfaces. JUCE, SQLite, the production notation renderer, and instrument/sample engines will be integrated in their designated phases without coupling them to the education logic.

See `docs/ARCHITECTURE.md`, `docs/PHASE_STATUS.md`, and the canonical build documents in `docs/spec`.
