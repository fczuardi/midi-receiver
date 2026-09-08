# Repository Guidelines

## Project Structure & Module Organization

This repository is a PlatformIO/Arduino experiment for M5 BLE MIDI receiver
firmware. The first hardware target was M5StickC Plus2; keep the receiver app
portable enough to build for M5Stack Core Gray when possible. The repository
root is an umbrella for packages, apps, CI consumers, and documentation.

- `packages/ble-midi-input/` contains the reusable `EmbeddedMusicBleMidiInput`
  PlatformIO package and package-owned tests.
- `apps/ble-midi-receiver-local-test/` contains the M5 display/diagnostic
  receiver firmware and app-owned tests.
- `ci/consumers/` contains package consumer fixtures for CI.
- `docs/devlog/` contains one narrative Markdown file per experiment slice.

Keep BLE MIDI parsing, display rendering, and connection state logic separated
enough that each can be tested or inspected independently. Do not add a second
receiver app for a board when only compile-time board settings differ.

## Build, Test, and Development Commands

Use PlatformIO commands from the repository root with `-d` pointing at the
package or app:

- `pio test -d packages/ble-midi-input -e native` runs package tests.
- `pio test -d apps/ble-midi-receiver-local-test -e native` runs app tests.
- `pio run -d apps/ble-midi-receiver-local-test` builds all app firmware
  environments.
- `pio run -d apps/ble-midi-receiver-local-test -e m5stick-cplus2` builds the
  M5StickC Plus2 firmware.
- `pio run -d apps/ble-midi-receiver-local-test -e m5stack-core-gray` builds the
  M5Stack Core Gray firmware.
- `pio run -d apps/ble-midi-receiver-local-test -e <env> -t upload` flashes the
  connected device for the selected environment.
- `pio device monitor -d apps/ble-midi-receiver-local-test -e <env>` opens
  serial logs for BLE connection and MIDI event debugging.

The local app sets BLE advertised names at compile time per environment:
`M5 Plus2 MIDI RX` for `m5stick-cplus2` and `M5 Gray MIDI RX` for
`m5stack-core-gray`.

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
