# 038 Test the pending MIDI event queue

## Goal

Cover the handoff between NimBLE callbacks and the application loop without
simulating the BLE stack or relying on hardware.

## Design

`PendingMidiEventQueue` now owns the fixed-capacity buffer, its mutex, event
ordering, drop counter, and clear operation. `BleMidiInput` still translates
parsed MIDI messages and drains the queue during `update()`, but no longer
owns the queue mechanics directly.

The queue remains an internal implementation unit of `ble-midi-input`. It is
not a new PlatformIO package or a public application contract.

## Verification

Native package tests cover:

- FIFO ordering and draining;
- capacity and drop counting;
- clearing pending events and drop counters.

The complete package and app native suites passed with 17 test cases. Firmware
builds for M5StickC Plus2 and M5Stack Core Gray remain the relevant compile
checks. Hardware behavior was not revalidated in this slice.
