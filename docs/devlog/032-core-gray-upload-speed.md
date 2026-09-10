# Core Gray upload speed

## Goal

Make the guarded Core Gray upload reliable after the build and board probe have
already succeeded.

## Observation

With `upload_speed = 1500000`, esptool connected to the ESP32 and started the
stub flasher, but the chip stopped responding while PlatformIO verified the
flash connection after changing baud rate. This is the same failure previously
observed in the Core Gray AMY and umbrella showcase experiments.

## Change

The Core Gray environment now uses `upload_speed = 460800`. The Plus2 remains
at `1500000` because this slice has no evidence that its upload path needs to
change.

## Verification

- The Core Gray firmware build completed successfully before this change.
- The failing upload reached the flash verification stage, confirming that the
  board guard and compiled firmware were not the cause.
- Physical re-upload at `460800` remains to be confirmed by the developer.
