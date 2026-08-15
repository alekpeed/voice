# Testing and QA

## Unit tests
Intervals, pitch classes, guide tones, motion classification, assignment cost, nearest-voicing search, transposition.

## Fixture tests
Canonical MIDI fixtures: C6->Ddim7; ii-V-I; guide-tone ii-V-I; common tones; crossing; ambiguous assignment; rolled chord; sustain transitions.

## Property tests
Transposition preserves interval structure; voice count remains valid; solver satisfies hard constraints; deterministic seeds reproduce exercises.

## Audio tests
Note on/off, sustain, repeated notes, polyphony, preset changes, missing samples, device reconnect, CPU spikes, xruns/dropouts.

## Screenshot QA
Notation, voice graph, keyboard, Barry Harris family view, Instrument Workshop. Notation proportions are a dedicated release gate.

## Human musical QA
A pianist must review whether feedback is sensible, alternatives are valid, jazz mode is not overly classical, Barry Harris examples are correct, and instruments feel playable.

## Regression policy
Every music-engine bug gets: minimal fixture -> failing test -> fix -> permanent regression test.

## Content QA
No duplicate IDs, no orphan exercises, valid prerequisites, valid book links, valid transposition, defined competency rules.
