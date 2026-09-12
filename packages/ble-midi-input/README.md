# ble-midi-input

Reusable BLE MIDI input transport package for embedded music experiments.

The package owns BLE-MIDI callbacks, pending event buffering, and conversion to
shared `NoteEvent`, `PitchBendEvent`, and disconnection notifications. It does
not own display state, receiver diagnostics, or audio output.

Registry consumers can install the latest published version:

```ini
lib_deps =
  fcz2/ble-midi-input
```

For reproducible firmware builds, pin an exact package version instead, for
example `fcz2/ble-midi-input@0.3.0`.

Consumers can override the advertised BLE name at compile time:

```ini
build_flags =
  -D BLE_MIDI_DEVICE_NAME='"M5 Gray MIDI RX"'
```

The package implements the BLE MIDI GATT receiver directly on top of
NimBLE-Arduino 2.x. MIDI channels are reported as the raw status nibble value,
`0..15`. Its internal MIDI parser currently handles Note On, Note Off, Control
Change, and Pitch Bend messages with BLE MIDI timestamps and running status. It
is intentionally not a complete MIDI 1.0 parser; unsupported system messages
are rejected rather than exposed as partially interpreted events.

Applications that register a `BleMidiInputDiagnosticSink` also receive the raw
status byte for unsupported channel or system messages. This is diagnostic
visibility only: unsupported messages are not queued and are never delivered to
the instrument-event sink.

Call `begin()` once before entering the application loop and call `update()`
regularly from that loop. Only one `BleMidiInput` instance may be active at a
time because NimBLE callbacks are routed through a shared active-instance
pointer.
