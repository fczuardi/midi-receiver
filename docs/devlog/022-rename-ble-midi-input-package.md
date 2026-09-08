# Slice 022: Rename BLE MIDI Input Package

The reusable BLE MIDI input package no longer uses the temporary
`EmbeddedMusic` prefix. The PlatformIO package name now follows the same
kebab-case convention as the package directory:

- `ble-midi-input`

The path remains `packages/ble-midi-input`, and the public C++ class was
already named `BleMidiInput`, so this slice only changes PlatformIO package
identity and the places that consume or package it.

Updated consumers:

- local receiver app dependency aliases;
- CI package consumer fixture;
- package archive filename in CI and `just consumer-build`;
- README and repository contributor notes.

Validation targets:

```bash
just test
just build
just consumer-build
```

The older `EmbeddedMusicFirmwareContracts` dependency name was left for a
separate naming cleanup slice and later renamed to `firmware-contracts`.
