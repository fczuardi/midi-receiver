# 039 Diagnose unsupported MIDI status bytes

## Goal

Make the receiver useful when investigating controller behavior outside the
small MIDI subset currently used by the package. In particular, a controller
channel switch might emit a Program Change, and the receiver previously gave no
indication that it had seen such a status byte.

## Design

`MidiMessageParser` now reports rejected MIDI messages through a diagnostic hook.
`BleMidiInput` places up to three raw MIDI bytes in its existing fixed-capacity
handoff queue and forwards them from `update()` to
`BleMidiInputDiagnosticSink`. The local receiver app logs the hexadecimal bytes,
command family, and channel when the status is a channel message. This keeps
serial output and application callbacks out of the BLE callback context.

The hook does not change the supported MIDI contract. Program Change, Channel
Pressure, Polyphonic Pressure, system messages, and realtime statuses remain
unsupported, are not placed in the pending event queue, and do not reach an
`InstrumentEventSink`.

The parser test verifies that complete unsupported messages are observable while
no decoded MIDI message is produced. It also keeps an incomplete message from
being reported as if it were complete.

## Verification

- `pio test -e native` in `packages/ble-midi-input`: 17 test cases passed;
- `pio test -e native` in `apps/ble-midi-receiver-local-test`: 16 test cases
  passed;
- local M5Stack Core Gray firmware build passed after refreshing the cached
  local package;
- on hardware, BLE MIDI Engineer generated `Program Change` messages for
  Distortion Guitar and Electric Grand. The receiver logged
  `raw=C0 1E` and `raw=C0 02`, respectively, confirming both the status and
  program payload on channel 0;
- the new `0.3.1` package has not yet been published to the PlatformIO
  Registry.

The Core Gray build reported 122,771 bytes of IRAM used (93.67%).
