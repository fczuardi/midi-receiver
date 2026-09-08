#!/usr/bin/env bash
set -euo pipefail

mode="report"
expected_board="${1:-}"
port="${2:-}"

if [[ "$expected_board" == "--port-for" ]]; then
  mode="port"
  expected_board="${2:-}"
  port="${3:-}"
fi

esptool_python="/home/fcz/.local/share/pipx/venvs/platformio/bin/python"
esptool_py="/home/fcz/dev/m5stick/.platformio-home/packages/tool-esptoolpy/esptool.py"

usage() {
  printf 'Usage: %s [m5stick-cplus2|m5stack-core-gray] [serial-port]\n' "$0" >&2
  printf '       %s detect [serial-port]\n' "$0" >&2
  printf '       %s --port-for [m5stick-cplus2|m5stack-core-gray] [serial-port]\n' "$0" >&2
}

first_serial_port() {
  local candidate=""

  if [[ -d /dev/serial/by-id ]]; then
    candidate="$(find /dev/serial/by-id -maxdepth 1 -type l | sort | head -n 1)"
    if [[ -n "$candidate" ]]; then
      printf '%s\n' "$candidate"
      return
    fi
  fi

  find /dev -maxdepth 1 \( -name 'ttyACM*' -o -name 'ttyUSB*' \) | sort | head -n 1
}

detect_board_from_probe() {
  local probe_output="$1"

  if grep -Eq 'ESP32-PICO|PICO-D4|PICO-V3' <<<"$probe_output"; then
    printf 'm5stick-cplus2\n'
    return
  fi

  if grep -Eq 'ESP32-D0WD|D0WDQ6|D0WD-V3|D0WD' <<<"$probe_output"; then
    printf 'm5stack-core-gray\n'
    return
  fi

  printf 'unknown\n'
}

if [[ -z "$expected_board" ]]; then
  usage
  exit 64
fi

case "$expected_board" in
  detect|m5stick-cplus2|m5stack-core-gray)
    ;;
  *)
    usage
    exit 64
    ;;
esac

if [[ -z "$port" ]]; then
  port="$(first_serial_port)"
fi

if [[ -z "$port" ]]; then
  printf 'No serial port found under /dev/serial/by-id, /dev/ttyACM*, or /dev/ttyUSB*.\n' >&2
  exit 66
fi

if [[ ! -e "$esptool_python" ]]; then
  printf 'PlatformIO Python interpreter not found: %s\n' "$esptool_python" >&2
  exit 69
fi

if [[ ! -e "$esptool_py" ]]; then
  printf 'esptool.py not found: %s\n' "$esptool_py" >&2
  exit 69
fi

if ! probe_output="$("$esptool_python" "$esptool_py" --port "$port" chip_id 2>&1)"; then
  printf '%s\n' "$probe_output" >&2
  exit 67
fi

detected_board="$(detect_board_from_probe "$probe_output")"

if [[ "$mode" == "report" ]]; then
  printf '%s\n' "$probe_output"
  printf 'Detected board family: %s\n' "$detected_board"
fi

if [[ "$expected_board" == "detect" ]]; then
  [[ "$detected_board" != "unknown" ]]
  exit $?
fi

if [[ "$detected_board" != "$expected_board" ]]; then
  if [[ "$mode" == "port" ]]; then
    printf '%s\n' "$probe_output" >&2
  fi
  printf 'Refusing upload: expected %s but detected %s on %s.\n' \
    "$expected_board" "$detected_board" "$port" >&2
  printf 'Set M5_SKIP_BOARD_GUARD=1 to bypass this check deliberately.\n' >&2
  exit 65
fi

if [[ "$mode" == "port" ]]; then
  printf '%s\n' "$port"
fi
