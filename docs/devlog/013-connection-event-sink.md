# Slice 013: Connection Event Sink

`NoteEventSink` gives another component a clean way to receive Note On/Off
events, but disconnection is a separate lifecycle event. If a MIDI sender
disconnects while notes are held, an instrument consumer also needs to clear its
own sounding state.

This slice adds `ConnectionEventSink` with one method: `onDisconnected()`.
`BleMidiPeripheral` can register this optional sink with
`setConnectionEventSink()`. The receiver does not own the sink; callers must
keep it alive while the receiver may use it.

Disconnection remains outside `NoteEvent` because it is not a note. When the BLE
disconnect callback is processed in `update()`, the receiver discards pending
MIDI activity, clears active notes in `AppState`, and then notifies the
connection sink.

Verification command:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio test -e native
env PLATFORMIO_CORE_DIR=.platformio-home pio run
```

Expected hardware result: reconnect behavior remains unchanged, while future
instrument integration can call `stopAll()` when the receiver reports
disconnection.
