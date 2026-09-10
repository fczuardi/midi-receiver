# Slice 027: NimBLE Scan Response Name

## Goal

Make the configured BLE MIDI device name visible to raw BLE scanners when the
NimBLE backend also advertises the 128-bit BLE MIDI service UUID.

## Problem

The previous NimBLE advertised-name slice called:

```cpp
_advertising->setName(deviceName);
```

but a Linux `btmon` trace of the Core Gray AMY app showed no name in either the
primary advertisement or scan response:

```text
Advertising Data[21]:
  Flags
  128-bit Service UUIDs

Scan response:
  Data length: 0
```

The primary legacy BLE advertisement is limited to 31 bytes. BLE MIDI already
uses 21 bytes for flags plus the 128-bit service UUID, so names such as
`M5 Gray AMY` cannot fit there with their BLE field overhead.

## Design

Enable scan-response data before setting the name in the package-owned NimBLE
backend:

```cpp
_advertising->addServiceUUID(service->getUUID());
_advertising->enableScanResponse(true);
_advertising->setName(deviceName);
```

With NimBLE-Arduino, `setName()` places the name in scan-response data when
scan response is enabled. This keeps the primary advertisement focused on the
BLE MIDI service UUID while giving active scanners a place to read the device
name.

The package version is bumped to `ble-midi-input@0.1.4` so consumers can pin
this scan-response behavior separately from the earlier `0.1.3` advertised-name
attempt.

## Verification Target

```bash
pio test -e native
pio run -d apps/ble-midi-receiver-local-test -e m5stick-cplus2
pio run -d apps/ble-midi-receiver-local-test -e m5stack-core-gray
```

Hardware follow-up should use `btmon` while scanning and confirm that the ESP32
scan response contains `Name (complete)` with the configured device name.
