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

The package uses a package-owned copy of BLE-MIDI's ESP32 NimBLE backend by
default. The local copy keeps the BLE-MIDI 2.2 backend aligned with
NimBLE-Arduino 2.x callback signatures, so modern pioarduino consumers do not
need app-level BLE transport fixes.

A consumer can opt back into the classic ESP32 BLE backend with:

```ini
build_flags =
  -D BLE_MIDI_INPUT_USE_CLASSIC_ESP32_BLE
```
