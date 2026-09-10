# Slice 024: NimBLE as the Default BLE Backend

## Goal

Make `ble-midi-input` more plug-and-play for modern consumers. The AMY synth
probe uses the newer pioarduino ESP32 platform, where BLE-MIDI's classic ESP32
backend no longer compiles cleanly because the Arduino-ESP32 BLE API changed.

## Design

`BleMidiInput` now selects BLE-MIDI's ESP32 NimBLE backend by default. Consumers
no longer need to know about this backend choice for the common path.

The older classic ESP32 BLE backend remains available behind an explicit build
flag:

```ini
-D BLE_MIDI_INPUT_USE_CLASSIC_ESP32_BLE
```

BLE-MIDI 2.2 expects the older NimBLE-Arduino security wrapper. Current
PlatformIO dependency resolution installs NimBLE-Arduino 2.x, where that wrapper
was removed. The package owns a tiny compatibility shim so each showcase does
not have to patch or vendor BLE-MIDI.

The receiver app build environments also move to pioarduino so the package is
validated first in the modern platform stack we expect new compositions to use.

## Validation

Commands:

```sh
pio test -e native
pio run -e m5stack-core-gray
pio run -e m5stick-cplus2
```

Results:

- native tests passed: 16 test cases;
- Core Gray firmware build passed with the default NimBLE backend;
- M5StickC Plus2 firmware build passed after explicitly mapping
  `board_build.variant = m5stack_stickc_plus2` for pioarduino.

Existing umbrella showcases should be repinned to this receiver commit and
rebuilt after this package slice lands.
