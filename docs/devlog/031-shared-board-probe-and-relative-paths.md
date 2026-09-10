# Shared board probe and relative paths

## Goal

Remove the duplicate board-family probe and eliminate machine-specific paths
from the local development commands.

## Design

`embedded-music-experiments/scripts/probe-esp32-board.sh` is the canonical
workspace tool. The receiver upload helper locates it through
`M5_WORKSPACE_TOOLS_DIR`, defaulting to the sibling umbrella checkout. The
override keeps the command usable with a different workspace layout without
making the package repository depend on `/home/fcz` or another username.

Package archives are now written to the receiver repository's `.tmp/`
directory. The consumer recipe converts that path to an absolute `file://`
URL only at execution time, because PlatformIO needs a concrete local archive
path while the repository itself should remain portable.

## Verification

- `bash -n scripts/upload-receiver.sh scripts/test-upload-guard-shell.sh`
- `scripts/test-upload-guard-shell.sh`
- `just pack`
- `just consumer-build`
- `just probe-board` reaches the hardware probe without the former
  `rich_click` mismatch; hardware availability remains environment-dependent.

## Boundary

The umbrella owns generic workspace tooling. The receiver remains a standalone
package host; its package and CI do not require the sibling checkout merely to
build or test the BLE MIDI library.
