# pioarduino platform migration

The isolated ESP32 package consumer now uses the same pioarduino
`platform-espressif32` release as the receiver applications. The native test
consumer remains on `platformio/native` where it does not require an ESP32
framework.

This removes the last official ESP32 platform from the receiver repository and
avoids mixing incompatible `tool-esptoolpy` versions in the shared PlatformIO
cache. The physical receiver applications were already using pioarduino; the
Core Gray upload speed remains `460800` and Plus2 remains `1500000`.

## Hardware observation

After the migration, the Core Gray firmware was uploaded with `just
upload-gray` and observed with `just monitor-gray`. The validated behavior was
preserved: BLE connect and disconnect events, Note On and Note Off, pitch bend,
and Control Change sustain events were all received and reported correctly.
