# Slice 021: Guarded Upload Recipes

The board-specific upload recipes now call `scripts/upload-receiver.sh`, which
runs a read-only ESP32 probe before flashing. The guard uses PlatformIO's
installed `esptool.py` to query the chip over serial and classifies the output
as one of the supported receiver targets:

- `m5stick-cplus2` for ESP32 PICO package strings;
- `m5stack-core-gray` for ESP32 D0WD package strings.

If the connected board does not match the selected recipe, the upload exits
before flashing. The validated serial port is also passed to PlatformIO with
`--upload-port`, so the guard and upload use the same device when more than one
serial port is present.

Observed hardware probe output matched those classifiers:

- Core Gray: `ESP32-D0WDQ6`, CP2104 USB serial bridge;
- StickC Plus2: `ESP32-PICO-V3-02`, `1a86` USB serial bridge.

`just probe-board` exposes the read-only probe directly. `M5_SKIP_BOARD_GUARD=1`
can bypass the guard for deliberate manual recovery, but normal uploads should
use the guarded path.

An initial one-line just recipe bug allowed PlatformIO to continue after a
guard refusal because the failed command substitution was not explicitly
handled. Upload flow now lives in `scripts/upload-receiver.sh`, and
`scripts/test-upload-guard-shell.sh` verifies that `pio` is not called after a
simulated guard refusal.

Validation targets:

```bash
bash -n scripts/probe-esp32-board.sh scripts/upload-receiver.sh scripts/test-upload-guard-shell.sh
just --list
just --dry-run upload-plus2
just --dry-run upload-gray
just test-upload-guard-shell
just probe-board
```

The last command requires a connected ESP32 device. The exact probe strings
should be confirmed once with each target board and the classifier tightened if
the real output differs from the expected package names.
