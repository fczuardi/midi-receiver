# Slice 018: Core Gray Receiver Build

The next umbrella showcase will use the same BLE MIDI receiver package on the
M5Stack Core Gray. Before composing that showcase with an instrument and speaker
output, this slice checks whether the existing local receiver firmware is
portable beyond the original M5StickC Plus2 target.

The receiver behavior stays unchanged: it initializes M5Unified, advertises BLE
MIDI, receives MIDI activity, updates `AppState`, and draws diagnostic display
state. The change is only PlatformIO targeting and device-neutral wording.

`platformio.ini` now builds two default firmware environments:

- `m5stick-cplus2`
- `m5stack-core-gray`

The serial boot banner now says `M5 BLE MIDI receiver` and logs the M5Unified
board id, so the same firmware source can be used on either board without
claiming it is always a Plus2.

The CI package consumer was also moved out of inline YAML into
`ci/consumers/ble-midi-input`, matching the checked-in fixture style used by
the shared instrument packages. The fixture consumes the packed archive through
`BLE_MIDI_INPUT_PACKAGE` instead of pointing back at the repository root from
inside the repository.

Validation targets:

```bash
pio test -e native
pio run -e m5stick-cplus2
pio run -e m5stack-core-gray
pio run
pio pkg pack . --output /home/fcz/dev/m5stick/.tmp
env BLE_MIDI_INPUT_PACKAGE=file:///home/fcz/dev/m5stick/.tmp/EmbeddedMusicBleMidiInput-0.1.0.tar.gz pio run -d ci/consumers/ble-midi-input
```

Hardware target:

```bash
pio run -e m5stack-core-gray --target upload
pio device monitor -e m5stack-core-gray
```
