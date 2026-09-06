# Slice 011: Typed Note Event Boundary

The receiver milestone proved that BLE MIDI note messages can be parsed,
displayed, and tracked as active notes. The next integration goal is to let an
instrument consume those note facts without inheriting BLE, display, or receiver
state details.

This slice adds a small `NoteEvent` contract to the receiver. It contains only
note data: event type, channel, note, and velocity. Pitch Bend and Control
Change remain separate message categories and are intentionally not included in
this note event type.

`BleMidiPeripheral` now turns parsed Note On/Off callbacks into `NoteEvent`
values before updating `AppState`. Serial logs label those messages as
`note_event`, making the producer boundary visible during hardware tests.

The receiver and buzzer projects currently duplicate this contract on purpose.
It is still experimental; once both sides use it successfully in an integration
firmware, we can decide whether it belongs in a shared library.

Verification command:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio test -e native
env PLATFORMIO_CORE_DIR=.platformio-home pio run
```

Expected hardware result: pressing and releasing keys over BLE still updates
the display and active-note list, while serial logs now include typed
`note_event` lines.
