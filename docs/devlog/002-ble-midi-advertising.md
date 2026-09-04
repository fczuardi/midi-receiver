# Slice 002: BLE MIDI Advertising

The second slice introduces Bluetooth Low Energy without parsing MIDI messages
yet. The testable goal is for the M5StickC Plus2 to advertise itself as a BLE
MIDI peripheral, accept a connection, and show the connection state on screen.

This slice is intentionally narrow. BLE MIDI has several separate concerns:
advertising, GATT services, connection callbacks, packet format, timestamps, and
MIDI message parsing. Combining all of those in one commit would make failures
harder to explain. Here, we only prove that another device can discover and
connect to our firmware.

The firmware is split by responsibility:

- `AppState` stores the current facts the app cares about.
- `DisplayView` owns all display drawing.
- `BleMidiPeripheral` owns BLE setup, the BLE MIDI service, and connection
  callbacks.
- `main.cpp` wires those parts together and keeps the loop readable.

The BLE MIDI service UUID is:

```text
03B80E5A-EDE8-4B33-A751-6CE34EC4C700
```

The BLE MIDI data characteristic UUID is:

```text
7772E5DB-3868-4112-A1A9-F2669D106BF3
```

The characteristic is configured with read, write, write-without-response, and
notify properties so it has the shape expected by BLE MIDI clients. Actual
incoming packet handling is left for the next slice.

The main C++ design choice in this slice is that BLE callbacks do not directly
redraw the screen. They only record that a connection event happened. The main
loop then applies that event through `BleMidiPeripheral::update()`, making the
control flow easier to read and review.

Acceptance test:

```bash
pio run -t upload --upload-port /dev/ttyACM0
pio device monitor --port /dev/ttyACM0
```

Expected behavior: the screen shows `BLE: advertising`, a BLE scanner or MIDI
app can find `M5 BLE MIDI RX`, and connecting changes the screen and serial log
to `connected`.

Hardware result: tested with the My Midi Hub Android app. The app discovered
`M5 BLE MIDI RX`, connected successfully, disconnected successfully, and the
firmware returned to advertising after each disconnect. Serial logs showed the
expected connect/disconnect sequence.
