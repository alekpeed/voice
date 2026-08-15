# Acoustic Piano Sample Packs

Voice Leading Lab does not commit or redistribute piano recordings. Users supply a legally obtained SFZ/WAV library, and the code remains independent of the recordings.

## Phase 2 compatibility

The loader supports the SFZ features needed for a conventional multi-velocity acoustic piano:

- `<control>`, `<global>`, `<group>`, and `<region>` inheritance.
- `default_path`, `sample`, `key`, `lokey`, `hikey`, and `pitch_keycenter`.
- `lovel`, `hivel`, `tune`, `volume`, `pan`, and `ampeg_release`.
- `trigger=attack` and `trigger=release` regions.
- Mono or stereo RIFF/WAVE samples in PCM 16/24/32-bit or IEEE float 32-bit format.

Quoted paths are supported. Sample files are resolved relative to the SFZ file and its `default_path`.

## Factory preset intent

The first production preset is `concert-grand-natural`. Register that ID with the path to a properly licensed multi-layer grand-piano SFZ. The engine preloads its WAV regions, applies velocity calibration, renders at the device sample rate, supports release regions, and keeps released notes sounding while the sustain pedal is held.

Future phases may add an optional pack installer, disk streaming for very large libraries, sympathetic resonance, half-pedal sample layers, una corda, and round-robin opcodes. These must not change the instrument interface.

## Play from MIDI

```sh
./build/voice-leading-lab --list-midi
./build/voice-leading-lab --play-piano /path/to/piano.sfz
```

The play command uses the first detected raw-MIDI device and the default ALSA output. To select them explicitly:

```sh
./build/voice-leading-lab --play-piano /path/to/piano.sfz /dev/snd/midiC2D0 hw:1,0
```

Press Enter to stop. Device disconnects generate synthetic note and pedal releases before audio shutdown.
