# Slice 006: Using the BLE-MIDI Library

The raw packet slice proved that MIDI data was reaching the device. The next
question was whether we should keep parsing BLE MIDI packets ourselves or join
the existing Arduino MIDI ecosystem.

For this slice, we chose the library path. The firmware now uses
`lathoub/BLE-MIDI`, which provides BLE MIDI transport for the FortySevenEffects
Arduino MIDI Library. That gives us parsed MIDI callbacks instead of making our
own BLE MIDI parser.

The app still keeps its own structure:

- `BleMidiPeripheral` wraps the library and owns callback registration.
- `AppState` stores the parsed activity the rest of the firmware cares about.
- `DisplayView` renders a compact view of the latest MIDI activity.
- `main.cpp` remains the small orchestration layer.

One C++ detail in this slice is that the MIDI library uses plain function
pointer callbacks. Those callbacks cannot capture a C++ object directly, so
`BleMidiPeripheral` uses a small static bridge: the library calls a static
function, and that function forwards to the active `BleMidiPeripheral` instance.

The display now reports parsed message activity:

- total MIDI messages;
- last activity type;
- note and channel for Note On/Off;
- velocity for Note On/Off.

The firmware also registers an Active Sensing callback, so the repeated `0xFE`
traffic observed in the raw packet logs is now counted as MIDI activity without
being mistaken for note input.

PlatformIO installs multiple BLE-MIDI transport dependencies, but this firmware
uses the ESP32 backend. The unused ArduinoBLE, Arduino_SpiNINA, and NimBLE
packages are ignored in `platformio.ini` to keep the build focused.

Hardware result: tested with the Android BLE MIDI sender. Keys and MIDI channels
were parsed and displayed correctly through the library callbacks. Pitch bend was
not displayed, which is expected for this slice because the firmware has not
registered a pitch bend callback yet.
