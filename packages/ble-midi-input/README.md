# ble-midi-input

Reusable BLE MIDI input transport package for embedded music experiments.

The package owns BLE-MIDI callbacks, pending event buffering, and conversion to
shared `NoteEvent`, `PitchBendEvent`, and disconnection notifications. It does
not own display state, receiver diagnostics, or audio output.

Consumers can override the advertised BLE name at compile time:

```ini
build_flags =
  -D BLE_MIDI_DEVICE_NAME='"M5 Gray MIDI RX"'
```

The package uses BLE-MIDI's ESP32 NimBLE backend by default. That backend is the
portable path for modern Arduino-ESP32 cores. A consumer can opt back into the
classic ESP32 BLE backend with:

```ini
build_flags =
  -D BLE_MIDI_INPUT_USE_CLASSIC_ESP32_BLE
```
