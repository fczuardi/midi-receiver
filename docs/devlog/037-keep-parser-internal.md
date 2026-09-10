# 037 Keep parser responsibilities internal

## Goal

Correct the premature package boundary introduced for the BLE MIDI parser.

## Design

The parser is no longer a separate PlatformIO package. `BleMidiPacketDecoder`
owns BLE MIDI packet framing, timestamps, and packet-local running status.
`MidiMessageParser` owns the generic MIDI channel-message state machine and
emits transport-independent `MidiMessage` values.

Both units remain independent of Arduino, NimBLE, audio, and application
contracts. They are internal implementation units of `ble-midi-input`, with
native tests covering each responsibility.

The previously published `ble-midi-packet-parser@0.1.0` was removed from the
PlatformIO Registry before this redesign. No replacement parser package is
published. A future package should wait for a real second consumer and broader
protocol scope.

## Verification

Native tests, package packing, the isolated consumer, and both firmware builds
must pass. Hardware behavior was not revalidated in this refactor.
