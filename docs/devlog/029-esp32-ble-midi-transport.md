# Slice 029: ESP32 BLE MIDI Transport

## Goal

Replace the patched `lathoub/BLE-MIDI` transport with a cleaner ESP32-focused
BLE MIDI dependency.

The previous stack worked, but only after this repository carried a
package-local NimBLE backend and consumer `lib_ignore` entries. That was useful
for learning, but it left the package with compatibility code that existed only
to keep an older transport shape alive.

## Change

`ble-midi-input@0.2.0` now depends directly on:

```text
h2zero/NimBLE-Arduino@2.5.1
```

The package no longer exports or builds the vendored BLE-MIDI NimBLE backend.
It also does not depend on `max22/ESP32-BLE-MIDI`, because that published
PlatformIO package includes a `src/main.cpp` example that defines
`setup()`/`loop()` and therefore cannot be consumed cleanly as a library.

Instead, `BleMidiInput` creates the standard BLE MIDI service and
characteristic directly with NimBLE-Arduino, parses incoming BLE MIDI packets,
and emits the small message set this project actually consumes: Note On,
Note Off, Control Change, and Pitch Bend.

The direct NimBLE implementation does not currently force explicit bonding.
Previous hardware validation used app-level BLE MIDI connections without
needing package-owned security policy, so that behavior is left to the BLE MIDI
central and NimBLE defaults until hardware testing says otherwise.

The local app and isolated consumer no longer ignore ArduinoBLE,
Arduino_SpiNINA, or other unused BLE stacks.

The package contract now reports MIDI channels exactly as delivered by the MIDI
status byte nibble:

```text
0..15
```

This is a deliberate contract change. The project is still experimental, and a
cleaner representation is more valuable than compatibility with the previous
`1..16` display-oriented interpretation.

Pitch Bend remains converted into the shared centered event value:

```text
-8192..8191
```

The Active Sensing diagnostic path was removed. The new transport does not
expose that callback, and keeping a dead application state field would make the
receiver claim a behavior it no longer observes.

## Boundaries

`BleMidiInput` still owns BLE transport callbacks and conversion to shared
instrument events. It does not know about display state, audio output, or
instrument policy.

`BleMidiPeripheral` remains app-local display and serial diagnostics.

`InstrumentEventSink`, `NoteEvent`, and `PitchBendEvent` stay in
`firmware-contracts`.

## Verification

Command checks:

```bash
just test
pio pkg pack packages/ble-midi-input --output /home/fcz/dev/m5stick/.tmp
PLATFORMIO_SETTING_ENABLE_TELEMETRY=no pio run -d apps/ble-midi-receiver-local-test -e m5stack-core-gray
PLATFORMIO_SETTING_ENABLE_TELEMETRY=no pio run -d apps/ble-midi-receiver-local-test -e m5stick-cplus2
PLATFORMIO_SETTING_ENABLE_TELEMETRY=no BLE_MIDI_INPUT_PACKAGE=file:///home/fcz/dev/m5stick/.tmp/ble-midi-input-0.2.0.tar.gz pio run -d ci/consumers/ble-midi-input
```

The native package and app tests passed. `pio pkg pack` produced
`ble-midi-input-0.2.0.tar.gz`.

Both app firmware targets built successfully:

- `m5stack-core-gray`;
- `m5stick-cplus2`.

The isolated consumer resolved the packed package and installed its transitive
dependencies, including `firmware-contracts` and `h2zero/NimBLE-Arduino`.
The local run then stopped in this machine's PlatformIO Python 3.14 environment
because `tool-esptoolpy` could not import `intelhex` while creating the
bootloader. The GitHub workflow uses Python 3.12 and remains the clean
validation target for that consumer path.

Hardware validation is still required before publishing `ble-midi-input@0.2.0`
to the PlatformIO Registry. The important observations are advertised name,
connect/reconnect, note on/off, velocity, pitch bend, disconnect cleanup, and
the downstream AMY panic behavior.
