# Backlog

## Validate ESP32-BLE-MIDI Transport On Hardware

`ble-midi-input@0.2.0` now implements the BLE MIDI receiver directly on
NimBLE-Arduino instead of carrying a vendored backend for `lathoub/BLE-MIDI`.
The codebase is cleaner: there is no package-local compatibility layer for an
older transport, no dependency on the non-library-clean
`max22/ESP32-BLE-MIDI` package, and consumers no longer need `lib_ignore`
entries to avoid unrelated BLE stacks.

Before publishing this version to the PlatformIO Registry, validate the
hardware-visible BLE behavior:

- advertised names still appear through scan response;
- connect, reconnect, disconnection cleanup, notes, velocity, pitch bend, and
  panic behavior work with the existing umbrella and AMY consumers;
- M5StickC Plus2 and M5Stack Core Gray firmware still build.

Do not publish `ble-midi-input@0.2.0` until the hardware scan/connect behavior
has been revalidated.

## Explore An esp-rs Receiver After PlatformIO Stabilizes

After the current PlatformIO/C++ package path is stable, published, and
validated through the umbrella showcases, start a separate Rust investigation
using `esp-rs`.

Keep this out of the current `midi-receiver` package until it proves value. A
good first shape is a sibling repository, for example `ble-midi-input-rs` or
`midi-receiver-rs-probe`, with a deliberately small scope:

- boot an ESP32 firmware with the Rust toolchain;
- advertise a BLE MIDI service with a clear device name;
- receive raw BLE characteristic writes and log packet bytes;
- parse Note On/Off only after raw packet reception is proven;
- compare the resulting boundaries with `ble-midi-input`, without trying to
  preserve PlatformIO/C++ compatibility.

The goal is a fresh view of the receiver module outside pioarduino, not a
replacement project until the current package ecosystem has a stable baseline.

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
