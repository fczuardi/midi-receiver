# Slice 007: Active Note Tracking

After proving that the BLE-MIDI library can parse Note On and Note Off events,
the next milestone is chord awareness. A MIDI receiver must know not only the
last key event, but also which notes are still held.

This slice adds an `ActiveNotes` module. It tracks active `(channel, note)`
pairs, which matters because the same note number can be held independently on
different MIDI channels.

The tracker handles common edge cases:

- duplicate Note On does not double-count a note;
- Note Off for an inactive note does not make the count negative;
- eight simultaneous notes are tracked;
- disconnect cleanup clears every held note.

The BLE callback bridge now keeps a bounded queue of pending note events instead
of storing only the most recent one. That is important for chords: if several
notes arrive before the main loop processes them, each Note On still reaches
`AppState`.

The display stays compact. It shows the active note count next to velocity and
lists up to eight currently held note numbers on a `Held` row.

This is also the first slice with native unit tests. `ActiveNotes` has no BLE,
Arduino, or display dependency, so PlatformIO can test it on the development
machine.

Verification commands:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio test -e native
env PLATFORMIO_CORE_DIR=.platformio-home pio run
```

Hardware result: tested on the M5StickC Plus2 with the BLE MIDI sender. The
display tracked simultaneous held keys and updated the active note list/count as
notes were pressed and released.
