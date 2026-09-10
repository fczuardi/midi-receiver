# 035 Extract the BLE MIDI packet parser

## Goal

Make the byte-level BLE MIDI interpretation independently testable without
Arduino or NimBLE.

## Design

`BleMidiPacketParser` now owns BLE MIDI packet framing, timestamps, running
status, and the supported channel messages: note on, note off, control change,
and pitch bend. It emits small typed `BleMidiMessage` values through a sink.

`BleMidiInput` remains responsible for NimBLE callbacks, timestamps for
diagnostics, asynchronous buffering, and conversion to the shared instrument
events. No universal MIDI or audio abstraction was introduced.

The parser remains local rather than adding the FortySevenEffects MIDI library.
That library is a useful MIDI stream parser, but it does not remove the BLE
MIDI packet framing work and would add another dependency to this package.

## Verification

The native package tests cover note messages, running status, control change,
center pitch bend, truncated packets, and unsupported messages. The package
and firmware consumer builds must also continue to pass.

Hardware behavior is unchanged by this slice and was not revalidated here.
