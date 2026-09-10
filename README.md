# midi-receiver

`midi-receiver` hosts `ble-midi-input`, a small PlatformIO package for turning
BLE MIDI input into reusable embedded music events.

The package is meant to be one independent module in a growing set of
composition-friendly firmware building blocks. A firmware can use it to receive
BLE MIDI notes, velocity, control changes, pitch bend, and disconnection events
without also adopting this repository's display demo or any audio backend.

The broader composition playground lives in
[embedded-music-experiments](https://github.com/fczuardi/embedded-music-experiments).
That umbrella project combines independent modules like this receiver with
instrument policy, tone output, AMY synthesis, and board-specific showcases.

## Package

The publishable package lives at:

```text
packages/ble-midi-input
```

The package keeps its BLE packet decoder and generic MIDI message parser as
independent, native-testable internal units. They are not separate PlatformIO
packages yet; their scope can grow as another consumer requires more of the
protocol.

It provides:

- BLE MIDI advertising and connection handling on ESP32/NimBLE;
- typed `NoteEvent` delivery through `InstrumentEventSink`;
- typed `PitchBendEvent` delivery;
- Control Change diagnostics for application-level observers;
- bounded callback-to-loop event buffering;
- disconnection notification so instruments can clear stuck notes.

`ble-midi-input` depends on the shared contracts from `firmware-contracts`,
owned by the umbrella repository. MIDI channels are reported as raw MIDI
status-nibble values, `0..15`.

## Install

Once the current package version is published to the PlatformIO Registry:

```ini
lib_deps =
  fcz2/ble-midi-input@0.3.0
```

During development, consumers can point at the package directory or a packed
tarball:

```ini
lib_deps =
  ble-midi-input=file://../../packages/ble-midi-input
```

## Apps

This repository also keeps a local receiver app at:

```text
apps/ble-midi-receiver-local-test
```

The app is a hardware validation surface for the package. It displays BLE
connection state and recent MIDI activity on supported M5Stack devices, but it
is not the reusable contract.

Current firmware targets:

- M5StickC Plus2;
- M5Stack Core Gray.

## Development

Common checks:

```bash
just test
just pack
just build-plus2
just build-gray
```

Upload and monitor helpers:

```bash
just probe-board
just upload-plus2
just monitor-plus2
just upload-gray
just monitor-gray
```

Development history, design tradeoffs, and hardware observations are recorded
in [docs/devlog](docs/devlog).
