# Slice 026: NimBLE Advertised Name

## Goal

After moving `ble-midi-input` to the NimBLE backend, Android SynthBridge showed
generic targets such as `Bluetooth MIDI`, and another scan showed `-` instead
of the configured firmware names. The BLE MIDI service still connected, but the
advertising metadata was less useful than before.

## Design

The package-owned NimBLE backend already called:

```cpp
BLEDevice::init(deviceName);
```

That sets the GAP device name, but it does not guarantee that scanner apps see
the local name in the advertisement or scan response. The backend now also calls:

```cpp
_advertising->setName(deviceName);
```

NimBLE-Arduino places the name in scan response data when scan response is
enabled, falling back to the advertisement payload otherwise. This keeps the
service UUID advertisement and makes the configured `BLE_MIDI_DEVICE_NAME`
observable to scanners that read advertised local names.

## Validation

Commands:

```sh
cd packages/ble-midi-input
pio test -e native

cd ../../apps/ble-midi-receiver-local-test
pio run -e m5stack-core-gray
pio run -e m5stick-cplus2
```

Expected hardware observation:

- BLE MIDI should still connect and deliver MIDI data;
- generic BLE scanners should have a better chance of showing the configured
  local name instead of `-`;
- MIDI-specific Android apps may still display a service label such as
  `Bluetooth MIDI`.
