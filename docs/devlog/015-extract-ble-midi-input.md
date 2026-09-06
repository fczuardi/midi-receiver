# Slice 015: Extract BLE MIDI Input

The receiver and buzzer instrument now share the same `NoteEvent` and
`InstrumentEventSink` contracts. Before composing BLE input with buzzer audio,
the receiver needs one more internal separation: transport should not be tied
directly to `AppState`, display, or serial logs.

This slice adds `BleMidiInput`, a BLE-MIDI transport component that owns the
library callbacks, pending event queue, MIDI parser polling, and Note On/Off
normalization. It can notify an `InstrumentEventSink` directly and has an
optional diagnostics observer for receiver-only visibility.

`BleMidiPeripheral` remains the firmware-facing receiver class. It now wraps
`BleMidiInput` and implements the diagnostics observer to update `AppState` and
print the same serial logs as before. This preserves the existing receiver
behavior while making the reusable input boundary easier to extract later.

The split keeps responsibilities explicit:

- `BleMidiInput`: BLE-MIDI transport and event production;
- `MidiNoteEventFactory`: parsed MIDI note fields to normalized `NoteEvent`;
- `BleMidiPeripheral`: receiver display state and serial diagnostics;
- `InstrumentEventSink`: optional downstream consumer boundary.

`BleMidiInput` still depends on Arduino and BLE-MIDI, so it is validated by the
firmware build rather than native unit tests. Native tests continue to cover the
pure state and note-event normalization code.

Verification command:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio test -e native
env PLATFORMIO_CORE_DIR=.platformio-home pio run
```

Expected hardware result: BLE advertising, connection logs, MIDI diagnostics,
active-note display, CC, Pitch Bend, and disconnect cleanup remain unchanged.
