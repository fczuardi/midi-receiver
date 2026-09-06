# Backlog

## Extract A Pure Pending Event Dispatcher

`BleMidiPeripheral::update()` currently drains pending BLE MIDI activity,
updates `AppState`, logs diagnostics, and notifies optional sinks. The current
native tests cover the small sink helpers, but they do not exercise the full
dispatch path inside `BleMidiPeripheral` because that class depends on Arduino,
BLE-MIDI globals, and `millis()`.

Revisit this when integration with an audio consumer makes automated coverage
more valuable. A good extraction would move the queue-draining rules into a pure
C++ unit that can be tested without BLE or display code.

Important behavior to cover:

- dispatch several pending note events in the original receive order;
- deliver already-normalized events to `NoteEventSink`;
- discard pending note events before notifying disconnection;
- avoid delivering any note event after disconnection;
- keep Control Change and Pitch Bend outside the note-event boundary.

## Define Overflow Policy For Audio Consumers

The receiver queue currently holds 32 pending MIDI events. When it overflows,
the firmware counts and logs dropped events. That was acceptable for the
diagnostic receiver milestone, but it matters more once a sink controls sound:
if a Note Off is dropped, an instrument could keep sounding until a later
cleanup event.

Revisit this before relying on the receiver as a live audio source. Possible
policies:

- prioritize Note Off events when the queue is full;
- notify connection/instrument sinks to call `stopAll()` after overflow;
- separate musical note delivery from display diagnostics;
- increase queue capacity only if tests show it is enough for real bursts.
