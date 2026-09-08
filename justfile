export PLATFORMIO_SETTING_ENABLE_TELEMETRY := "no"
export TMPDIR := "/home/fcz/dev/m5stick/.tmp"

app := "apps/ble-midi-receiver-local-test"
package := "packages/ble-midi-input"
consumer := "ci/consumers/ble-midi-input"
archive := "/home/fcz/dev/m5stick/.tmp/BleMidiInput-0.1.1.tar.gz"
board_guard := "scripts/probe-esp32-board.sh"
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
    {{uploader}} m5stick-cplus2 m5stick-cplus2

upload-gray:
    {{uploader}} m5stack-core-gray m5stack-core-gray

probe-board:
    {{board_guard}} detect

monitor-plus2:
    pio device monitor -d {{app}} -e m5stick-cplus2

monitor-gray:
    pio device monitor -d {{app}} -e m5stack-core-gray

pack:
    mkdir -p /home/fcz/dev/m5stick/.tmp
    pio pkg pack {{package}} --output /home/fcz/dev/m5stick/.tmp

consumer-build: pack
    BLE_MIDI_INPUT_PACKAGE=file://{{archive}} pio run -d {{consumer}}
