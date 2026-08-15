# Voice Leading Lab

Desktop-first piano training software centered on voice paths rather than chord labels.

## Current milestone

Phases 0-5 are implemented. The project provides subsystem contracts, app-shell state, logging, settings, deterministic tests, CI, Linux raw-MIDI input, resilient MIDI sessions, a polyphonic SFZ/WAV acoustic-piano engine, velocity calibration, low-latency Linux ALSA output, configurable sonority detection, weighted 2-4 voice assignment, a voice-path visualizer, and compact grand-staff SVG notation with chord symbols, accidentals, ledger lines, voice highlights, fingering, scaling, wrapping, and playback cursor. The included study PDFs are indexed as the canonical curriculum: 64 voice-leading concepts, 80 companion exercises, 12 companion etudes, and all 33 Barry Harris guide chapters. Harmonic analysis, persistence, and interactive curriculum implementations begin in later phases.

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
./build/voice-leading-lab --visualizer-smoke
./build/voice-leading-lab --notation-smoke
```

Export the deterministic ii-V-I voice-graph fixture as SVG:

```sh
./build/voice-leading-lab --export-visualizer-svg /path/to/voice-paths.svg
```

Export the same fixture as compact grand-staff notation:

```sh
./build/voice-leading-lab --export-notation-svg /path/to/notation.svg
```

Play an installed SFZ piano from a connected MIDI keyboard:

```sh
./build/voice-leading-lab --play-piano /path/to/piano.sfz
```

The first detected Linux raw-MIDI input and default ALSA output are used. Optional explicit device arguments are documented in `docs/SAMPLE_PACKS.md`.

## Architecture

Subsystem boundaries are defined under `include/vll`. The user interface and audio technology remain replaceable behind interfaces. JUCE, SQLite, desktop notation widgets, and instrument/sample engines can be integrated without coupling them to the education logic.

See `docs/ARCHITECTURE.md`, `docs/PHASE_STATUS.md`, `docs/SAMPLE_PACKS.md`, `docs/PDF_CURRICULUM_ALIGNMENT.md`, and the canonical build documents in `docs/spec`.
