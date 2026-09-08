# Slice 023: Rename Firmware Contracts Consumer

The shared event contract package was renamed in the umbrella repository from
`EmbeddedMusicFirmwareContracts` to `firmware-contracts`.

This receiver now consumes the kebab-case package name everywhere active:

- the `ble-midi-input` package metadata;
- package native test dependencies;
- local receiver app native test dependencies;
- the CI archive consumer fixture;
- README current-state text.

Validation targets:

```bash
just test
just build
just consumer-build
```

Older devlog entries keep historical names from their original slices.
