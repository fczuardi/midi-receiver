#!/usr/bin/env bash
set -euo pipefail

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

cat > "$tmpdir/guard" <<'SCRIPT'
#!/usr/bin/env bash
printf 'simulated guard refusal\n' >&2
exit 65
SCRIPT

cat > "$tmpdir/pio" <<'SCRIPT'
#!/usr/bin/env bash
printf 'pio should not run after guard refusal\n' >&2
exit 99
SCRIPT

chmod +x "$tmpdir/guard" "$tmpdir/pio"

set +e
BOARD_GUARD="$tmpdir/guard" PIO="$tmpdir/pio" \
  scripts/upload-receiver.sh m5stack-core-gray m5stack-core-gray
status=$?
set -e

if [[ "$status" != "65" ]]; then
  printf 'Expected guard refusal exit 65, got %s\n' "$status" >&2
  exit 1
fi
