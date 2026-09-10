# ble-midi-input

Reusable BLE MIDI input transport package for embedded music experiments.

The package owns BLE-MIDI callbacks, pending event buffering, and conversion to
shared `NoteEvent`, `PitchBendEvent`, and disconnection notifications. It does
not own display state, receiver diagnostics, or audio output.

Registry consumers can install the package directly once the matching version is
published:

```ini
lib_deps =
  fcz2/ble-midi-input@0.2.0
```

Consumers can override the advertised BLE name at compile time:

```ini
build_flags =
  -D BLE_MIDI_DEVICE_NAME='"M5 Gray MIDI RX"'
```

The package implements the BLE MIDI GATT receiver directly on top of
NimBLE-Arduino 2.x. MIDI channels are reported as the raw status nibble value,
`0..15`.
