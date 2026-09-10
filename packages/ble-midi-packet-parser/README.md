# ble-midi-packet-parser

Small PlatformIO package for decoding BLE MIDI packet framing.

Version `0.1.0` is intentionally incomplete. It supports BLE MIDI timestamps,
running status, Note On/Off, Control Change, and Pitch Bend. SysEx, MIDI
realtime messages, and other MIDI message types are not supported yet.

The parser has no Arduino, BLE-stack, dynamic-allocation, or audio dependency.
It emits `BleMidiMessage` values through `BleMidiMessageSink` so applications
can translate them into their own event contracts.

The scope can grow incrementally as real consumers require more of BLE MIDI.
