# Slice 020: Justfile Receiver Recipes

The package/app layout made the correct PlatformIO commands longer and easier
to mistype, especially for board-specific upload targets. This slice adds a
root `justfile` as a small command facade.

The recipes keep one receiver app and route board differences through the
existing PlatformIO environments:

- `just upload-plus2` flashes `m5stick-cplus2`, advertising as
  `M5 Plus2 MIDI RX`;
- `just upload-gray` flashes `m5stack-core-gray`, advertising as
  `M5 Gray MIDI RX`.

The same file also exposes matching build, monitor, test, package, and
consumer-build recipes so day-to-day commands stay discoverable.

Validation targets:

```bash
just --list
just test
just build
just consumer-build
```

Hardware validation remains manual with either `just upload-plus2` or
`just upload-gray`, depending on the connected device.
