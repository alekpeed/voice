# Master Prompt for Coding Agent

You are implementing Voice Leading Lab, a desktop-first piano training application.

Read every document in this pack before writing production code.

Priority: correctness -> MIDI/audio reliability -> musical validity -> maintainable architecture -> legible visualization/notation -> UX polish -> AI last.

Do not jump phases. Follow 08_BUILD_PHASES.md.

At each phase: inspect repo; restate objective; identify dependencies; write/update tests where practical; implement smallest coherent vertical slice; run tests; fix regressions; update docs if architecture changed; commit cleanly.

Keep MIDI, performance state, voice assignment, harmonic analysis, voice-leading evaluation, exercises, audio, notation, UI, persistence, and AI separate.

Audio is first-class. Do not leave toy oscillator sounds as the permanent implementation. Architect for realistic acoustic piano, Rhodes, and Wurlitzer, including instrument-specific tuning, velocity, EQ, FX, and presets.

The central object is the voice path. Chord labels are descriptive metadata. Preserve enough structured data to answer what each voice did, what stayed, semitone/step/leap movement, crossing, guide-tone behavior, and available alternatives.

Notation must be compact and conventional. Never stretch a short phrase across excess space. Never display a generic keyboard unless it highlights musically relevant notes.

Barry Harris features must use the same voice engine: major/minor sixth-diminished, inversions, borrowing, related dominants, minor-6/dominant relationships, passing diminished movement, melody harmonization, and a free lab.

Every music-analysis bug requires a regression fixture. Exercise generation must be seed-reproducible.

After each phase report only: implemented; tests passing; known limitations; next phase.

First action: Phase 0 only. Create repo architecture, build system, CI/test skeleton, subsystem interfaces, and app shell. Do not implement AI or Barry Harris content in Phase 0.
