# Repository Guidelines

## Project Structure & Module Organization

This repository is a PlatformIO/Arduino experiment for M5 BLE MIDI receiver
firmware. The first hardware target was M5StickC Plus2; keep the receiver app
portable enough to build for M5Stack Core Gray when possible. Use the standard
PlatformIO layout:

- `platformio.ini` for board, framework, monitor speed, and pinned dependencies.
- `src/` for application firmware, with `main.cpp` as the entry point.
- `include/` for shared project headers.
- `lib/` for local libraries only when code is reused across modules.
- `test/` for PlatformIO unit tests.
- `data/` only if filesystem assets become necessary.

Keep BLE MIDI parsing, display rendering, and connection state logic separated enough that each can be tested or inspected independently.

## Build, Test, and Development Commands

Use PlatformIO commands from the repository root:

- `pio run` builds the firmware for the configured default environments.
- `pio run -e m5stick-cplus2` builds the M5StickC Plus2 firmware.
- `pio run -e m5stack-core-gray` builds the M5Stack Core Gray firmware.
- `pio run -t upload` flashes the connected default target.
- `pio device monitor` opens serial logs for BLE connection and MIDI event debugging.
- `pio test` runs PlatformIO tests when `test/` exists.

If multiple environments are introduced, document the default one in `platformio.ini` and use `pio run -e <env>`.

## Coding Style & Naming Conventions

Write Arduino C++ with 2-space indentation and clear, small functions. Prefer explicit names such as `activeNoteCount`, `handleNoteOn`, and `renderConnectionStatus`. Use `PascalCase` for types/classes, `camelCase` for variables and functions, and `UPPER_SNAKE_CASE` only for constants or macros.

Keep dependency versions explicit in `platformio.ini`. Prefer M5Unified for display and button access, as noted in the README.

## Testing Guidelines

Prioritize tests for MIDI byte parsing, active-note tracking, disconnect cleanup, and reconnection behavior. Name tests after the behavior being verified, for example `test_note_off_clears_active_note`. Hardware behavior should also be validated manually with a BLE MIDI source and serial logs.

Before considering the milestone complete, verify the README success criteria, including eight simultaneous notes and at least ten minutes of continuous operation.

## Commit & Pull Request Guidelines

This repository uses `jj` instead of direct `git` commands for day-to-day version control. Use `jj status` to inspect changes and `jj commit -m "<message>"` to record work.

The existing history uses Conventional Commit style, for example `docs: describe the initial BLE MIDI receiver experiment`. Continue using short, imperative messages with a scope when useful: `firmware: display active note count`.

Pull requests should include a concise description, hardware tested, PlatformIO commands run, and any observed BLE MIDI source used for validation. Include photos or screenshots only when display behavior changes.
