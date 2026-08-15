# Voice Leading Lab

Desktop-first piano training software centered on voice paths rather than chord labels.

## Current milestone

Phases 0-12 are implemented. The project includes deterministic MIDI, audio, analysis, notation, feedback, curriculum, all-key exercises, canonical book integration, instrument-workshop presets, fixed-melody harmonization, and a reharmonization lab with independent voice locks. The study PDFs remain the canonical curriculum source.

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
./build/voice-leading-lab --phase-10-12-smoke
./build/voice-leading-lab --phase-13-15-smoke
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
