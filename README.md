# Voice Leading Lab

Desktop-first piano training software centered on voice paths rather than chord labels.

## Current milestone

Phases 0-9 are implemented. The project provides subsystem contracts, app-shell state, logging, settings, deterministic tests, CI, Linux raw-MIDI input, resilient MIDI sessions, a polyphonic SFZ/WAV acoustic-piano engine, velocity calibration, low-latency Linux ALSA output, configurable sonority detection, weighted 2-4 voice assignment, a voice-path visualizer, compact grand-staff SVG notation, exact deterministic feedback, the first complete two-voice ii-V-I course path, and seeded all-key exercise generation with nearest voicings, guide-tone mode, 2/3/4 voices, and fixed soprano or bass. The included study PDFs are the canonical curriculum: 64 stable voice-leading concept IDs route to their exact Study Guide pages, Practice Companion pages, units, and printed exercises. A deterministic prerequisite graph and five-level competency tracker record attempts, assistance, and transposed success. Harmonic recognition and durable persistence begin in later phases.

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
./build/voice-leading-lab --feedback-smoke
./build/voice-leading-lab --course-smoke
./build/voice-leading-lab --generator-smoke
./build/voice-leading-lab --book-smoke
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
