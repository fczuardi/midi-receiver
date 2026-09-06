# Slice 016: Package BLE MIDI Input

`BleMidiInput` is now separated from receiver display state, but separation
inside one repository is not enough for reuse. This slice turns the reusable
input layer into a PlatformIO library package named
`EmbeddedMusicBleMidiInput`.

The root `library.json` declares the package because Git dependencies install
the repository root. The manifest exports only the BLE input boundary, the note
event factory, this documentation, the README, and the manifest itself. It also
uses `build.srcFilter` so consumers compile only:

- `src/BleMidiInput.cpp`;
- `src/MidiNoteEventFactory.cpp`.

The package depends on `EmbeddedMusicFirmwareContracts` for `NoteEvent` and
`InstrumentEventSink`, and on `lathoub/BLE-MIDI@2.2` for the BLE-MIDI transport.
Consumers may still need the same `lib_ignore` entries used by this firmware to
select the ESP32 BLE backend cleanly.

Two limitations are explicit for now. First, the advertised BLE name is still
fixed as `M5 BLE MIDI RX` because the BLE-MIDI library constructs its global
transport through a static macro. Second, only one `BleMidiInput` instance may
be active because callbacks are routed through a single active instance pointer.

CI now validates the package in two ways: `pio pkg pack` checks the package
manifest/export rules, and a temporary PlatformIO consumer builds against
`file://${{ github.workspace }}` to prove another firmware can include
`BleMidiInput.h`.

Verification command:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio test -e native
env PLATFORMIO_CORE_DIR=.platformio-home pio run
env PLATFORMIO_CORE_DIR=.platformio-home pio pkg pack . --output /tmp
```

Expected result: native tests pass, receiver firmware builds, and the package
tarball contains only the reusable BLE input files and documentation.
