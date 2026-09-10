export PLATFORMIO_SETTING_ENABLE_TELEMETRY := "no"

app := "apps/ble-midi-receiver-local-test"
package := "packages/ble-midi-input"
consumer := "ci/consumers/ble-midi-input"
tmp_dir := ".tmp"
workspace_tools := env_var_or_default("M5_WORKSPACE_TOOLS_DIR", "../embedded-music-experiments")
board_guard := workspace_tools + "/scripts/probe-esp32-board.sh"
uploader := "scripts/upload-receiver.sh"

default:
    @just --list

test-package:
    pio test -d {{package}} -e native

test-app:
    pio test -d {{app}} -e native

test-upload-guard-shell:
    scripts/test-upload-guard-shell.sh

test: test-package test-app test-upload-guard-shell

build:
    pio run -d {{app}}

build-plus2:
    pio run -d {{app}} -e m5stick-cplus2

build-gray:
    pio run -d {{app}} -e m5stack-core-gray

upload-plus2:
    BOARD_GUARD="{{board_guard}}" {{uploader}} m5stick-cplus2 m5stick-cplus2

upload-gray:
    BOARD_GUARD="{{board_guard}}" {{uploader}} m5stack-core-gray m5stack-core-gray

probe-board:
    {{board_guard}} detect

monitor-plus2:
    pio device monitor -d {{app}} -e m5stick-cplus2

monitor-gray:
    pio device monitor -d {{app}} -e m5stack-core-gray

pack:
    mkdir -p {{tmp_dir}}
    pio pkg pack {{package}} --output {{tmp_dir}}

consumer-build: pack
    package_version="$(python3 -c 'import json; print(json.load(open("packages/ble-midi-input/library.json", encoding="utf-8"))["version"])')"; \
      archive="$(pwd)/{{tmp_dir}}/ble-midi-input-$package_version.tar.gz"; \
      test -f "$archive"; \
      BLE_MIDI_INPUT_PACKAGE="file://$archive" pio run -d {{consumer}}
