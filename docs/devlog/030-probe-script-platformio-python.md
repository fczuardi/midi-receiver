# Slice 030: Probe Script PlatformIO Python

## Goal

Make `just probe-board` use the shared PlatformIO Python environment instead of
the developer's global PlatformIO `pipx` interpreter.

## Problem

Following the README exposed a failure already seen in the umbrella repository:

```text
ModuleNotFoundError: No module named 'rich_click'
```

The receiver's probe script still had hardcoded paths:

```text
/home/fcz/.local/share/pipx/venvs/platformio/bin/python
/home/fcz/dev/m5stick/.platformio-home/packages/tool-esptoolpy/esptool.py
```

That mixes the shared PlatformIO package cache with a different Python
environment. If `tool-esptoolpy` expects modules installed in
`.platformio-home/penv`, invoking it with the `pipx` Python can fail before the
board probe even reaches the serial port.

## Change

`scripts/probe-esp32-board.sh` now mirrors the umbrella script:

- derives the workspace path from the script location;
- defaults `PLATFORMIO_CORE_DIR` to the shared `.platformio-home`;
- uses `${PLATFORMIO_PYTHON:-$pio_home/penv/bin/python}`;
- keeps `PLATFORMIO_CORE_DIR` and `PLATFORMIO_PYTHON` overrideable.

## Follow-Up

The same board-probe and guarded-upload scripts now exist in both
`midi-receiver` and `embedded-music-experiments`. That duplication is a real
candidate for future shared tooling, but the immediate fix keeps this repository
usable without adding a new package or cross-repository dependency.
