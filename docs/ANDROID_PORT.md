# Galaxy Tab Kotlin Port

## Target

- Native Kotlin and Jetpack Compose
- Samsung Galaxy Tab and comparable Android tablets
- Landscape-first three-pane workspace with navigation rail
- Minimum Android 9 (API 28); target/compile API 36

## Android boundaries

- `AndroidMidiInput` uses Android's `MidiManager`, including USB and Bluetooth
  MIDI devices exposed by the operating system.
- `ProgressStore` uses Preferences DataStore for device-local progress.
- The three canonical study PDFs ship in `app/src/main/assets/books` and are
  materialized privately by `BookAssets` when opened.
- Instrument definitions, EQ, tuning, curriculum, generation, evaluation,
  Barry Harris fields, ear prompts, and analysis are pure Kotlin.

## UI

The Compose shell provides Home, Learn, Practice, Voices, Lab, Barry Harris,
Ear Training, Free Play, Progress, and Sounds destinations. Galaxy Tab landscape
uses persistent navigation plus three simultaneous work panes.

## Legacy reference

The C++ implementation remains temporarily in the repository as a regression
reference. It is not the Android release target and uses no NDK bridge.
