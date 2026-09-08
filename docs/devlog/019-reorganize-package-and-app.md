# Slice 019: Reorganize Package And App

The receiver repository now follows the same package/app layout used by
`monophonic-instrument`.

The reusable BLE MIDI transport package moved to:

- `packages/ble-midi-input`

The M5 display and diagnostics firmware moved to:

- `apps/ble-midi-receiver-local-test`

Tests moved with their ownership boundary. `MidiNoteEventFactory` and shared
contract tests live under the package. `ActiveNotes`, `AppState`, and
`MidiNoteName` tests live under the local app.

The local app now sets the BLE advertised name at compile time:

- `m5stick-cplus2`: `M5 Plus2 MIDI RX`
- `m5stack-core-gray`: `M5 Gray MIDI RX`

This keeps one receiver app while making hardware scans unambiguous. The
package still has a default `M5 BLE MIDI RX` name for consumers that do not set
`BLE_MIDI_DEVICE_NAME`.

Validation targets:

```bash
pio test -d packages/ble-midi-input -e native
pio test -d apps/ble-midi-receiver-local-test -e native
pio run -d apps/ble-midi-receiver-local-test -e m5stick-cplus2
pio run -d apps/ble-midi-receiver-local-test -e m5stack-core-gray
pio run -d apps/ble-midi-receiver-local-test
pio pkg pack packages/ble-midi-input --output /home/fcz/dev/m5stick/.tmp
env BLE_MIDI_INPUT_PACKAGE=file:///home/fcz/dev/m5stick/.tmp/BleMidiInput-0.1.1.tar.gz pio run -d ci/consumers/ble-midi-input
```
