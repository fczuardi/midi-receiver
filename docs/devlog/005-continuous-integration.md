# Slice 005: Continuous Integration

After proving that BLE packet activity reaches the firmware, we added a CI slice
to protect the build while the codebase grows.

The CI workflow runs on pushes to `main` and on pull requests. It installs a
pinned PlatformIO version, builds the firmware, and uploads the generated binary
files as workflow artifacts.

This does not replace hardware testing. GitHub Actions cannot connect to the
M5StickC Plus2, scan over BLE, or verify Android app behavior. Those checks stay
manual and should continue to be recorded in these devlog chapters.

The value of CI is narrower but important:

- catch C++ compile errors;
- verify `platformio.ini` and the board manifest still work;
- verify pinned dependencies can still be resolved;
- keep generated firmware binaries available from each CI run.

The workflow uploads:

- `firmware.bin`
- `bootloader.bin`
- `partitions.bin`

Those files are not a polished release process yet. They are build artifacts:
useful for inspection and manual flashing, but not a promise of stable firmware.
Tagged GitHub Releases and a browser-based web flasher can come later, once MIDI
parsing and note tracking are more mature.

Local verification still uses the same command:

```bash
env PLATFORMIO_CORE_DIR=.platformio-home pio run
```

