#!/usr/bin/env bash
set -euo pipefail

ARDUINO_CLI="${ARDUINO_CLI:-arduino-cli}"
FQBN="${FQBN:-arduino:renesas_uno:minima}"
WARNINGS="${WARNINGS:-all}"
TCP1819_SRC="${TCP1819_SRC:-../TCP1819}"

if [[ ! -d examples ]]; then
  echo "No examples/ directory found"
  exit 1
fi

if [[ ! -d "${TCP1819_SRC}" ]]; then
  echo "TCP1819_SRC not found: ${TCP1819_SRC}"
  echo "Clone egp/TCP1819 alongside this repo or set TCP1819_SRC=/path/to/TCP1819"
  exit 1
fi

echo "==> Arduino CLI version"
"${ARDUINO_CLI}" version

echo "==> Updating index"
"${ARDUINO_CLI}" core update-index

echo "==> Installing core for ${FQBN%:*}"
"${ARDUINO_CLI}" core install "${FQBN%:*}"

sketches="$(find examples -type f -name '*.ino' | sort)"
if [[ -z "${sketches}" ]]; then
  echo "No example sketches found under examples/"
  exit 1
fi

count="$(printf '%s\n' "${sketches}" | sed '/^$/d' | wc -l | tr -d ' ')"
echo "==> Compiling ${count} sketch(es) for ${FQBN}"

printf '%s\n' "${sketches}" | while IFS= read -r sketch; do
  [[ -n "${sketch}" ]] || continue
  sketch_dir="$(dirname "${sketch}")"
  echo
  echo "---- ${sketch_dir}"
  "${ARDUINO_CLI}" compile \
    --fqbn "${FQBN}" \
    --warnings "${WARNINGS}" \
    --library . \
    --library "${TCP1819_SRC}" \
    "${sketch_dir}"
done