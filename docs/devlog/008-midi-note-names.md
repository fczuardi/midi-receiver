# Slice 008: MIDI Note Names

The active-note display worked, but raw MIDI numbers are not very friendly while
testing with a keyboard. This slice keeps numeric values in the serial log and
changes the screen to show musical note names.

The new `MidiNoteName` module is deliberately small and pure. It has no Arduino,
BLE, or display dependency, so native tests can verify the mapping quickly.

We use the common MIDI octave convention:

- MIDI note 60 is `C4`;
- MIDI note 69 is `A4`;
- MIDI note 0 is `C-1`;
- MIDI note 127 is `G9`.

Accidentals are shown with sharps, for example `C#4`. A MIDI note number does
not carry key-signature context, so the firmware cannot know whether the player
would spell that pitch as `C#` or `Db`. Sharps keep the display simple and avoid
depending on special font glyphs.

The display still stays compact. The `Note` row shows the latest note name and
channel, while the `Held` row lists up to five active note names sorted from low
to high. If more notes are held, the row ends with `+` to show that the visible
list is truncated.

Verification commands:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio test -e native
env PLATFORMIO_CORE_DIR=.platformio-home pio run
```
