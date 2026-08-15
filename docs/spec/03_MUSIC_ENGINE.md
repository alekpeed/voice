# Music and Voice-Leading Engine

## Principle
The central object is a voice path, not a chord symbol.

## Sonority detection
Handle human arpeggiation, rolled chords, small attack offsets, sustain, legato transitions, and repeated notes. Make chord-window timing configurable.

## Voice assignment
Use weighted matching rather than sorting alone. Cost terms: pitch distance, common-tone bonus, register continuity, held-note continuity, crossing penalty, soprano/bass/user locks, leap penalty. Return assignment plus ambiguity/confidence.

## Metrics
Per transition: total semitone displacement, max voice movement, stationary voices, semitone moves, whole-step moves, leaps, crossing, overlap, spacing, soprano contour, bass contour.

## Jazz default
Do not impose species-counterpoint rules by default. Priorities: satisfy musical constraints; guide-tone resolution; common-tone retention; efficient motion; register/spacing; avoid unnecessary leaps/crossing. Classical diagnostics may be optional.

## Guide tones
Identify 3rds/7ths and track their destinations through seventh-chord progressions.

## Nearest-voicing solver
Inputs: source voicing, target harmony, voice count, register bounds, locked voices, required top/bass, max leap, spacing rules. Output ranked candidates with movement cost and explanation metadata.

## Melody harmonization
Input fixed melody plus harmonic/style constraints. Return multiple valid harmonizations ranked by smoothness, voice independence, guide-tone logic, harmonic validity, and style. Never pretend one solution is uniquely correct when it is not.

## Inner-line mode
Allow a line to be defined first, e.g. C-B-Bb-A, then derive/support harmony while preserving that line.

## Feedback
Specific facts only. Example: “Alto moved F4 to B4, a tritone. F4 to E4 was available while preserving the destination harmony.” Avoid vague scores.

## Transposition
All suitable exercises must support all 12 keys, cycle of fourths, chromatic order, random order, and selected key pools.
