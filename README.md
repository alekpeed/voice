# Voice Leading Lab

Desktop-first piano training software centered on voice paths rather than chord labels.

## Current milestone

Phases 0-2 are implemented. The project provides subsystem contracts, app-shell state, logging, settings, deterministic tests, CI, Linux raw-MIDI input, resilient MIDI sessions, a polyphonic SFZ/WAV acoustic-piano engine, velocity calibration, and low-latency Linux ALSA output. Music analysis, notation, persistence, and curriculum implementations begin in later phases.

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

List Linux raw-MIDI inputs or run the deterministic MIDI smoke test:

```sh
./build/voice-leading-lab --list-midi
./build/voice-leading-lab --midi-smoke
./build/voice-leading-lab --audio-smoke
./build/voice-leading-lab --alsa-null-smoke
```

## Architecture

Subsystem boundaries are defined under `include/vll`. The user interface and audio technology remain replaceable behind interfaces. JUCE, SQLite, the production notation renderer, and instrument/sample engines will be integrated in their designated phases without coupling them to the education logic.

See `docs/ARCHITECTURE.md`, `docs/PHASE_STATUS.md`, `docs/SAMPLE_PACKS.md`, and the canonical build documents in `docs/spec`.
