# Backlog

## Spike: Evaluate ESP32-BLE-MIDI As The Default Transport

`lathoub/BLE-MIDI` was a useful bootstrap dependency, but the current package
now carries a vendored NimBLE backend to keep BLE-MIDI 2.2 compatible with
NimBLE-Arduino 2.x and pioarduino. That is a sign that the transport dependency
may no longer match the project direction.

Prioritize a spike to evaluate `max22-/ESP32-BLE-MIDI` as the default backend.
It is ESP32-focused, NimBLE-based, and appears closer to the hardware/platform
shape we are actually targeting.

Keep the public `BleMidiInput` contract stable during the spike if possible.
The evaluation should prove:

- consumer builds no longer need a vendored BLE-MIDI backend;
- `lib_ignore` can be reduced or removed without selecting the wrong BLE stack;
- advertised names still appear through scan response;
- connect, reconnect, disconnection cleanup, notes, velocity, pitch bend, and
  panic behavior remain compatible with the existing umbrella and AMY consumers;
- M5StickC Plus2 and M5Stack Core Gray firmware still build.

Do not publish a new Registry version from this spike until the hardware-visible
BLE behavior has been revalidated.

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
- deliver already-normalized events to `InstrumentEventSink`;
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
