# Control Change Forwarding

## Goal

Forward parsed MIDI Control Change messages to the shared instrument boundary
so a backend can interpret continuous controls such as the modulation wheel.

## Design

`BleMidiInput` now emits `ControlChangeEvent` through `InstrumentEventSink`
after delivering the same message to its diagnostic observer. The receiver
preserves zero-based MIDI channels and the original 7-bit controller/value
ranges. It does not assign meaning to CC1 or depend on AMY.

The package now depends on the matching `firmware-contracts` commit and moves
to version `0.4.0`, since forwarding Control Change is a new public event
surface. Existing consumers that ignore this optional event remain valid.

## Verification

Native receiver tests cover the shared event shape. The full receiver test
suite and the firmware build remain the next validation target after the
updated contract dependency is fetched by PlatformIO.
