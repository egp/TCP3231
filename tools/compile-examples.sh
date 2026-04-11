#!/usr/bin/env bash
set -euo pipefail

ARDUINO_CLI="${ARDUINO_CLI:-arduino-cli}"
FQBN="${FQBN:-arduino:renesas_uno:minima}"
WARNINGS="${WARNINGS:-all}"
DEPENDENCY_LIB="${DEPENDENCY_LIB:-BitBang_I2C}"

if [[ ! -d examples ]]; then
  echo "No examples/ directory found"
  exit 1
fi

echo "==> Arduino CLI version"
"${ARDUINO_CLI}" version

echo "==> Updating index"
"${ARDUINO_CLI}" core update-index
"${ARDUINO_CLI}" lib update-index

echo "==> Installing core for ${FQBN%:*}"
"${ARDUINO_CLI}" core install "${FQBN%:*}"

echo "==> Installing dependency library: ${DEPENDENCY_LIB}"
"${ARDUINO_CLI}" lib install "${DEPENDENCY_LIB}"

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
    "${sketch_dir}"
done