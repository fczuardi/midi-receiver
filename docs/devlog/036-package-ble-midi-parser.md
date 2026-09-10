# 036 Package the BLE MIDI parser

## Goal

Give the byte-level parser its own PlatformIO package boundary so other
projects can reuse it without depending on the BLE transport or Arduino.

## Design

`packages/ble-midi-packet-parser` now owns the parser, manifest, README, and
native tests. The package intentionally supports only timestamps, running
status, Note On/Off, Control Change, and Pitch Bend in version `0.1.0`.

`ble-midi-input` consumes it as a declared dependency. The receiver continues
to own NimBLE callbacks, asynchronous buffering, diagnostics, and translation
to the shared instrument-event contracts.

The package is intentionally small even though established projects such as
Control Surface and the older FortySevenEffects transport stack provide broader
BLE MIDI or MIDI parsing. This gives us a useful, testable package now without
claiming full protocol coverage.

## Verification

The parser package native tests passed. The receiver package tests and local
consumer build must pass after the parser package is available to PlatformIO.
The parser package has not yet been hardware-tested independently; receiver
hardware behavior was validated before this extraction.
