#!/usr/bin/env bash
set -euo pipefail

ARDUINO_CLI="${ARDUINO_CLI:-arduino-cli}"
FQBN="${FQBN:-arduino:renesas_uno:minima}"
WARNINGS="${WARNINGS:-all}"

SKETCHBOOK_LIBS="${HOME}/Arduino/libraries"
TCP1819_SRC="${TCP1819_SRC:-}"
TCP1819_DST="${SKETCHBOOK_LIBS}/TCP1819"
SIBLING_TCP1819="$(cd .. && pwd)/TCP1819"

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

mkdir -p "${SKETCHBOOK_LIBS}"

if [[ -n "${TCP1819_SRC}" ]]; then
  echo "==> Installing TCP1819 from TCP1819_SRC=${TCP1819_SRC}"
  rm -rf "${TCP1819_DST}"
  cp -R "${TCP1819_SRC}" "${TCP1819_DST}"
elif [[ -f "${TCP1819_DST}/src/TCP1819.h" ]]; then
  echo "==> Using installed TCP1819 at ${TCP1819_DST}"
elif [[ -f "${SIBLING_TCP1819}/src/TCP1819.h" ]]; then
  echo "==> Installing TCP1819 from sibling repo ${SIBLING_TCP1819}"
  rm -rf "${TCP1819_DST}"
  cp -R "${SIBLING_TCP1819}" "${TCP1819_DST}"
else
  echo "TCP1819 not found."
  echo "Set TCP1819_SRC=/path/to/TCP1819, or install it at ${TCP1819_DST}"
  exit 1
fi

if [[ ! -f "${TCP1819_DST}/src/TCP1819.h" ]]; then
  echo "Installed TCP1819 is missing src/TCP1819.h"
  exit 1
fi

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