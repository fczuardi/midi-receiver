# Versioned package consumer archive

## Goal

Keep package-consumer validation correct when `ble-midi-input` changes version.

## Change

The local `consumer-build` recipe and GitHub Actions CI now read the package
version from `packages/ble-midi-input/library.json` and construct the expected
`ble-midi-input-<version>.tar.gz` archive name. CI also verifies that the
archive exists before starting the consumer build.

Previously both paths embedded `0.2.0`, which made a normal package version
bump leave the consumer validation pointing at a stale filename.

## Boundary

This change only improves package-distribution validation. It does not alter
the BLE MIDI parser, transport, or exported API.

## Verification

- `just pack`
- `just consumer-build`
- CI workflow syntax and the consumer archive path are version-derived.
