# M5StickC Plus2 BLE MIDI receiver experiment

This repository contains a small experiment to determine whether an
M5StickC Plus2 can reliably receive MIDI messages over Bluetooth Low
Energy.

The experiment uses PlatformIO with the Arduino framework.

## Current milestone

Create firmware that:

- advertises the M5StickC Plus2 as a BLE MIDI device;
- accepts an incoming BLE MIDI connection;
- displays the connection status on its screen;
- receives and displays:
  - Note On;
  - Note Off;
  - MIDI channel;
  - note number;
  - velocity;
- keeps a count of currently active notes;
- clears active notes after disconnection.

No audio synthesis is required.

## Hardware

- M5StickC Plus2
- USB-C cable for power, flashing, and serial logs
- BLE MIDI source for testing

The BLE MIDI source may be a phone, tablet, or computer. A conventional
USB MIDI controller may also be connected to a phone or tablet through
USB OTG and routed to the M5StickC Plus2 over BLE MIDI.

## Success criteria

The milestone is complete when the device can:

1. connect and reconnect without rebooting;
2. receive Note On and Note Off events;
3. receive a chord with at least eight simultaneous notes;
4. display the received events without noticeable input delay;
5. avoid leaving notes active after a disconnection;
6. run continuously for at least ten minutes without crashing.

## Non-goals

This experiment does not include:

- audio synthesis;
- speakers or audio output;
- SysEx patch handling;
- a sequencer or arpeggiator;
- direct USB MIDI input;
- MIDI over TRS or DIN;
- product design or custom hardware.

## Development

Use PlatformIO with the Arduino framework.

Keep dependency versions explicit in `platformio.ini` so builds are
reproducible.

Prefer M5Unified for access to the display and buttons.

The firmware should produce useful serial logs for connection,
disconnection, and received MIDI events.
