# Slice 001: PlatformIO Baseline

The first goal was not BLE MIDI. It was proving that the toolchain, board, USB
connection, display, and serial logs all worked together.

This matters because embedded debugging gets expensive when several unknowns are
stacked at once. Before adding Bluetooth, we wanted a firmware image that could
boot reliably, draw text on the M5StickC Plus2 screen, and print simple progress
messages over USB serial.

We created the initial PlatformIO project structure:

- `platformio.ini` defines the build environment.
- `boards/m5stick-cplus2.json` describes the M5StickC Plus2 board for
  PlatformIO.
- `src/main.cpp` contains the first firmware entry point.
- `.gitignore` keeps generated PlatformIO files out of version control.

The first firmware initialized `M5Unified`, set up the display, printed boot
messages, and refreshed an uptime counter. After flashing, the device showed our
own screen instead of the factory cube demo, confirming that the full build and
upload loop was working.

The important lesson from this slice is that small embedded milestones should
remove uncertainty. At the end of this slice, we knew PlatformIO could build the
project, the M5StickC Plus2 could be flashed, and serial/display feedback were
available for the next layer of debugging.

Verification command:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio run
```

