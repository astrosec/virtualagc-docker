#!/bin/bash
# Fort Neptune Virtual AGC entrypoint.
#
# Mission software selection:
#   AGC_SOFTWARE    name of a baked-in rope image under /opt/agc/roms
#                   (default Luminary099, the Apollo 11 LM software).
#   AGC_IMAGE_PATH  absolute path to a mounted rope image; overrides
#                   AGC_SOFTWARE entirely for custom or experimental builds.
#   AGC_PORT        channel-I/O server port (default 19697).
#   AGC_EXTRA_ARGS  appended to the yaAGC command line verbatim.
#
# The container always starts from a clean machine state (--no-resume): a
# reproducible cold boot is the point of this packaging. Mount a volume over
# the working directory and drop --no-resume via AGC_EXTRA_ARGS if you
# deliberately want core-resume behavior.
set -euo pipefail

AGC_SOFTWARE="${AGC_SOFTWARE:-Luminary099}"
AGC_PORT="${AGC_PORT:-19697}"

if [[ -n "${AGC_IMAGE_PATH:-}" ]]; then
    rope="${AGC_IMAGE_PATH}"
else
    rope="/opt/agc/roms/${AGC_SOFTWARE}.bin"
fi

if [[ ! -r "${rope}" ]]; then
    echo "AGC rope image not found or unreadable: ${rope}" >&2
    echo "Baked-in mission software (see /opt/agc/roms/manifest.json):" >&2
    ls /opt/agc/roms/*.bin >&2 || true
    exit 1
fi

echo "yaAGC starting: rope=${rope} port=${AGC_PORT}"
# shellcheck disable=SC2086
exec /opt/agc/bin/yaAGC "--exec=${rope}" "--port=${AGC_PORT}" \
    --nodebug --no-resume --quiet ${AGC_EXTRA_ARGS:-}
