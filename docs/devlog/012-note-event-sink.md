# Slice 012: Note Event Sink

The previous slice made `NoteEvent` visible inside the receiver, but the event
was still consumed immediately by `AppState`. That proved the shape of the data,
but did not yet give another component a clean place to receive it.

This slice adds `NoteEventSink`, a small typed interface with one method:
`onNoteEvent(const NoteEvent& event)`. `BleMidiPeripheral` can now register an
optional sink with `setNoteEventSink()`. The receiver does not own that object;
the caller must keep it alive for at least as long as the receiver uses it.

When BLE MIDI Note On/Off callbacks are drained in the main update loop,
`BleMidiPeripheral` creates a `NoteEvent`, updates `AppState` as before, and
then notifies the optional sink. Display state, BLE details, raw parser data,
Control Change, and Pitch Bend stay out of this boundary.

The helper `notifyNoteEventSink()` centralizes the null check and is covered by
native tests with a fake sink. This keeps the contract testable without pulling
Arduino BLE code into the native test environment.

Verification command:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio test -e native
env PLATFORMIO_CORE_DIR=.platformio-home pio run
```

Expected hardware result: note logs and display behavior remain unchanged, and
future integration code can subscribe to typed note events without depending on
receiver display state.
