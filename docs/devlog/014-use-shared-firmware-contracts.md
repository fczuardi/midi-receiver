# Slice 014: Use Shared Firmware Contracts

The receiver originally introduced local contracts while the event boundary was
still being discovered: `NoteEvent`, `NoteEventSink`, and
`ConnectionEventSink`. The buzzer instrument has now validated the same boundary
from the consumer side, so this slice moves the receiver to the shared
`EmbeddedMusicFirmwareContracts` PlatformIO package.

`platformio.ini` declares the package from the umbrella repository, pinned to
the exact commit that published `NoteEvent` and `InstrumentEventSink`. The local
copies of `NoteEvent` and the receiver-specific sink interfaces were removed.

The receiver still owns MIDI normalization. `MidiNoteEventFactory` remains local
and turns parsed note fields into a shared `NoteEvent`, including the MIDI
convention that Note On with velocity zero becomes Note Off. This keeps raw MIDI
parsing rules on the producer side instead of pushing them into instruments.

`BleMidiPeripheral` now exposes a single optional `InstrumentEventSink`.
Note events call `onNoteEvent()`, and BLE disconnection calls
`onDisconnected()` after pending MIDI activity is discarded and display state is
cleared.

This slice does not integrate audio. Display diagnostics, Control Change, Pitch
Bend, active-note display state, and BLE transport behavior stay unchanged.

Verification command:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio test -e native
env PLATFORMIO_CORE_DIR=.platformio-home pio run
```

Expected result: native tests pass, firmware builds, and PlatformIO resolves
`EmbeddedMusicFirmwareContracts` from the pinned Git commit.
