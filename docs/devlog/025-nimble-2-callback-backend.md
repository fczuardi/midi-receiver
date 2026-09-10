# Slice 025: NimBLE 2 Callback Backend

## Goal

The AMY BLE synth probe could connect from Android, but the firmware did not
log `ble_midi: connected`. That meant the modern NimBLE path was building, but
the BLE-MIDI connection callbacks were not reaching `BleMidiInput`.

## Design

BLE-MIDI 2.2's ESP32 NimBLE backend still implements NimBLE-Arduino 1.x callback
signatures:

```cpp
onConnect(BLEServer*)
onDisconnect(BLEServer*)
onWrite(BLECharacteristic*)
```

NimBLE-Arduino 2.x calls signatures that include `NimBLEConnInfo`. Because the
old methods no longer override the active virtual callbacks, the server can be
connected while `BleMidiInput` never receives connection or write events.

`ble-midi-input` now carries a package-owned copy of the BLE-MIDI ESP32 NimBLE
backend with the NimBLE 2.x callback signatures. This is not an alternate
legacy backend. It is the modern default backend source adjusted to the modern
NimBLE API so consumers remain plug-and-play.

The package source includes a small `src/hardware/` forwarding header so
`BleMidiInput.cpp` deterministically includes the package-owned backend instead
of PlatformIO resolving the upstream BLE-MIDI header first.

## Validation

Commands:

```sh
cd packages/ble-midi-input
pio test -e native

cd /home/fcz/dev/m5stick/amy-synth-probe/apps/ble-midi-amy
pio run
```

Results:

- native package tests passed: 7 test cases;
- AMY BLE app build passed through the package-owned NimBLE backend;
- dependency output confirmed `ble-midi-input@0.1.2` after the local package
  was refreshed.

## Hardware Observation

The AMY BLE app connected from Android SynthBridge through the entry displayed
as `Bluetooth MIDI`. After connection, the Android virtual controller could send
notes and pitch-bend positions to the hardware.

The Android app did not show `M5 Gray AMY` in its scan UI. A Linux desktop scan
showed a cached-looking `M5 Gray Speaker` name, so device-name display appears
to depend on scanner/app caching and whether the tool presents the BLE device
name or the BLE-MIDI service label.
