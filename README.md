# M5 BLE MIDI receiver experiment

This repository documents a completed experiment to determine whether an
M5Stack device can reliably receive and interpret MIDI messages over Bluetooth
Low Energy. The first hardware target was the M5StickC Plus2; the local receiver
firmware also builds for the M5Stack Core Gray.

The experiment uses PlatformIO with the Arduino framework.

## Status

The initial receiver milestone was completed on September 5, 2026 and validated
on M5StickC Plus2 hardware. The same local receiver app now has build coverage
for the M5Stack Core Gray so downstream Core Gray instrument showcases can reuse
the same `EmbeddedMusicBleMidiInput` package.

The firmware:

- advertises the board as a BLE MIDI device;
- accepts an incoming BLE MIDI connection;
- displays connection status and received activity;
- receives and interprets:
  - Note On;
  - Note Off;
  - typed note events for instrument integration;
  - MIDI channel;
  - note number and musical note name;
  - velocity;
  - Control Change messages;
  - sustain state observed through CC 64;
  - Pitch Bend;
- tracks simultaneous active notes across MIDI channels;
- preserves bursts of events with a bounded queue;
- can deliver typed note events and disconnection to an optional
  `InstrumentEventSink`;
- clears pending events and active notes after disconnection.

`BleMidiInput` owns BLE-MIDI transport and shared event production.
`BleMidiPeripheral` keeps the receiver-specific display and serial diagnostics
on top of that input layer.

The root `library.json` packages `BleMidiInput` as
`EmbeddedMusicBleMidiInput`, so another PlatformIO firmware can consume the BLE
MIDI transport without importing this receiver's display application.

No audio synthesis is included.

Development history and hardware observations are recorded in
[docs/devlog](docs/devlog).

## Hardware

- M5StickC Plus2
- M5Stack Core Gray
- USB-C cable for power, flashing, and serial logs
- BLE MIDI source for testing

The BLE MIDI source may be a phone, tablet, or computer. A conventional
USB MIDI controller may also be connected to a phone or tablet through
USB OTG and routed to the M5StickC Plus2 over BLE MIDI.

## Completed success criteria

The milestone was validated with the following criteria:

- [x] connect and reconnect without rebooting;
- [x] receive Note On and Note Off events;
- [x] receive a chord with at least eight simultaneous notes;
- [x] display received events without noticeable input delay;
- [x] avoid leaving notes active after a disconnection;
- [x] run continuously for at least ten minutes without crashing.

Native tests cover pure MIDI state logic, and CI runs those tests before
building the firmware. Hardware-dependent BLE and display behavior remains
manually validated.

## Non-goals

This experiment does not include:

- audio synthesis;
- speakers or audio output;
- SysEx patch handling;
- a sequencer or arpeggiator;
- direct USB MIDI input;
- MIDI over TRS or DIN;
- product design or custom hardware.

Future experiments may reuse lessons or code from this repository, but this
milestone does not define a complete stable library API for downstream projects.
The current integration contract is the shared `EmbeddedMusicFirmwareContracts`
PlatformIO package.

## Prior art

BLE MIDI reception has previously been demonstrated on the original
M5Stack and M5StickC:

- [M5Stack BLE MIDI receiver (2018)](https://qiita.com/KazuyukiEguchi/items/e166ede5c97438b90187)
- [M5StickC BLE MIDI receiver (2020)](https://lang-ship.com/blog/work/m5stickc-esp32-ble-midi/)
- [Bidirectional M5StickC BLE MIDI experiment (2020)](https://pointofviewpoint.linclip.com/archives/3005)

These experiments establish basic feasibility. This repository focuses
on a reproducible M5StickC Plus2 implementation using current libraries,
structured MIDI event handling, connection recovery, and explicit
success criteria.

## Development

Use PlatformIO with the Arduino framework.

Keep dependency versions explicit in `platformio.ini` so builds are
reproducible.

Prefer M5Unified for access to the display and buttons.

The firmware produces serial logs for connection, disconnection, received MIDI
events, and bounded-queue overflow diagnostics.

Build and test locally with:

```bash
pio test -e native
pio run
pio run -e m5stick-cplus2
pio run -e m5stack-core-gray
pio pkg pack . --output /tmp
```
