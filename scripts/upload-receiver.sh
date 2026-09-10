#!/usr/bin/env bash
set -euo pipefail

expected_board="${1:-}"
pio_env="${2:-}"

app_dir="${APP_DIR:-apps/ble-midi-receiver-local-test}"
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd "$script_dir/.." && pwd)"
workspace_tools="${M5_WORKSPACE_TOOLS_DIR:-$repo_dir/../embedded-music-experiments}"
board_guard="${BOARD_GUARD:-$workspace_tools/scripts/probe-esp32-board.sh}"
pio_bin="${PIO:-pio}"

usage() {
  printf 'Usage: %s [m5stick-cplus2|m5stack-core-gray] [platformio-env]\n' "$0" >&2
}

if [[ -z "$expected_board" || -z "$pio_env" ]]; then
  usage
  exit 64
fi

if [[ "${M5_SKIP_BOARD_GUARD:-0}" == "1" ]]; then
  exec "$pio_bin" run -d "$app_dir" -e "$pio_env" --target upload
fi

port="$("$board_guard" --port-for "$expected_board")"
exec "$pio_bin" run -d "$app_dir" -e "$pio_env" --upload-port "$port" --target upload
