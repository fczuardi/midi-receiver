# Slice 028: PlatformIO Registry Publishing

## Goal

Publish `ble-midi-input` through the PlatformIO Registry so downstream
showcases can depend on a normal package instead of a pinned Git repository.

## Change

Added `.github/workflows/publish-platformio.yml` for tags shaped like:

```text
ble-midi-input-v*
```

The workflow verifies that the tag version matches
`packages/ble-midi-input/library.json`, runs the package native tests, packs the
package, builds the isolated consumer from the packed tarball, and publishes the
package with the `PLATFORMIO_AUTH_TOKEN` repository secret.

The existing CI and `just consumer-build` archive path were corrected from the
old `0.1.1` filename to the current `0.1.4` package version. The isolated
consumer now relies on the package manifest to resolve `firmware-contracts`
instead of declaring that dependency separately.

`NimBLE-Arduino` is also declared as a direct package dependency. The package
owns a NimBLE-specific BLE-MIDI backend and includes `NimBLEDevice.h` directly,
so relying on `BLE-MIDI` to pull NimBLE transitively made the isolated consumer
too easy to misconfigure.

## Rationale

The repository root remains a monorepo host, not a PlatformIO package. The
publishable artifact is the packed package subdirectory:

```text
packages/ble-midi-input
```

Using the Registry keeps umbrella showcases closer to plug-and-play dependency
resolution while preserving the package/app separation inside this repository.

## Verification

```bash
just test
pio pkg pack packages/ble-midi-input --output /tmp
just build
```

The firmware build was run after removing the app `.pio` directory, and both
firmware environments installed `ble-midi-input@0.1.4` from the local package
directory.

The isolated consumer resolved the packed `ble-midi-input@0.1.4` tarball and
its transitive `firmware-contracts` dependency, but the local build then stopped
in the local PlatformIO Python environment because `tool-esptoolpy` could not
import `intelhex`. The GitHub workflow uses a pinned Python 3.12 environment and
remains the validation target for that clean consumer build.

No new hardware behavior is introduced by this slice.
