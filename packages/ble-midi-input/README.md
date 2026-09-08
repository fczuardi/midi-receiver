# EmbeddedMusicBleMidiInput

Reusable BLE MIDI input transport package for embedded music experiments.

The package owns BLE-MIDI callbacks, pending event buffering, and conversion to
shared `NoteEvent`, `PitchBendEvent`, and disconnection notifications. It does
not own display state, receiver diagnostics, or audio output.

Consumers can override the advertised BLE name at compile time:

```ini
build_flags =
  -D BLE_MIDI_DEVICE_NAME='"M5 Gray MIDI RX"'
```
