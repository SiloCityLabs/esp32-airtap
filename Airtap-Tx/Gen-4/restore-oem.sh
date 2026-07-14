#!/usr/bin/env bash
# Restore Gen-4 OEM full-flash dump (already ciphertext).
# Device must be in download mode: hold IO9, pulse EN, release IO9.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
PORT="${1:-/dev/ttyUSB0}"
BIN="$ROOT/firmware-dump/oem-firmware-full.bin"

if [[ ! -f "$BIN" ]]; then
  echo "Missing $BIN" >&2
  exit 1
fi

source "$ROOT/../../.venv/bin/activate"
echo "Restoring OEM dump to $PORT (raw ciphertext write)..."
esptool --port "$PORT" --chip esp32c6 --before no-reset --after no-reset --baud 460800 \
  write-flash --flash-mode dio --flash-freq 80m --flash-size 8MB --force \
  --ignore-flash-enc-efuse \
  0x0 "$BIN"
echo "Done. Pulse EN (without holding IO9) to boot OEM."
