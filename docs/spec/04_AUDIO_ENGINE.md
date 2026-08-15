# Audio and Instrument Engine

## Priority
Audio realism is a release-level requirement, not polish. The app must not sound like a cheap MIDI trainer.

## Architecture
MIDI -> Instrument Engine -> Insert FX -> Master Bus -> Audio Device. Analysis receives MIDI separately.

## Acoustic piano
Use high-quality sampled or hybrid instruments with, where source material permits: multiple velocity layers, smooth layer transition, release samples, sustain resonance, sympathetic resonance, half-pedal, una corda, pedal/key noise, realistic decay, useful round robins, configurable velocity curves.

Factory sounds: Concert Grand Natural; Studio Grand Dry; Upright Warm; Felt/Intimate.

## Rhodes-style instruments
Treat as tunable instruments, not generic EP waveforms.
Quick controls: Tone/Brightness, Bell, Bark, Warmth, Tremolo Depth/Rate, Drive, Reverb.
Workshop controls where engine supports them: per-note tuning, tine/pickup relationship, hammer/attack character, pickup-distance equivalent, release/mechanical noise, velocity curve, stereo behavior, amp/cabinet, saturation, compressor, chorus, phaser, tremolo/pan, reverb, parametric EQ.

## Wurlitzer-style instruments
Reed/attack character, bark/drive, release/mechanical noise, velocity response, tremolo, amp/cabinet, EQ, compression, saturation, reverb.

## Equalizer
Per-instrument optional parametric EQ: high-pass, low shelf, 2-4 parametric bands, high shelf, output gain. Each parametric band has frequency, gain, Q, bypass.

## Tuning
Global A4 reference (default 440 Hz) and fine cents. Electric-piano presets may include per-note tuning. Audio tuning is rendering-only; the educational pitch engine still uses intended MIDI pitch.

## Velocity calibration
Per-device wizard: play softly, medium, loudly -> derive curve -> manual adjustment. Linear/soft/hard/custom curves and instrument-specific overrides.

## FX chain
Instrument -> preamp/drive -> EQ -> compressor -> modulation -> amp/cabinet -> reverb -> master. Bypassable.

## Licensing
Never commit commercial sample libraries without redistribution rights. Separate code, instrument definitions, and optional downloadable sample packs.

## QA
Validate soft/medium/hard attacks, decay, pedal, repeated notes, dense polyphony, low/high register, velocity transitions, release tails, CPU, latency, and dropout behavior.
