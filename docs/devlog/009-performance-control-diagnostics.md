# Slice 009: Performance Control Diagnostics

The receiver can now show note activity clearly, so the next useful MIDI
surface is performance controls. Many controllers send extra gestures such as
sustain, modulation, and pitch bend without using Note On or Note Off.

This slice keeps the receiver neutral. It observes and reports those controls,
but it does not decide which notes should keep sounding. That interpretation
belongs to a future synth or sound engine.

The firmware now records:

- `Control Change` messages, including channel, controller number, and value;
- CC 64 as the observed sustain/hold state;
- `Pitch Bend` messages as the centered signed value reported by the MIDI
  parser.

The display uses the existing compact rows. `Note` shows the latest note name,
channel, and velocity. `Ctrl` shows sustain state, the latest CC number/value,
and the latest pitch bend value. CC and pitch fields stay blank until the first
matching message arrives. The serial log still prints raw numeric fields, which
is better for identifying what a specific controller strip sends.

Expected discoveries during hardware testing:

- a sustain or hold button should usually send `CC64` with value `127` for on
  and `0` for off;
- a modulation strip often sends `CC1`, but the receiver reports the actual CC
  number instead of assuming the controller model;
- a pitch strip sends `Pitch Bend`, typically around `0` at center.

Verification commands:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio test -e native
env PLATFORMIO_CORE_DIR=.platformio-home pio run
```
