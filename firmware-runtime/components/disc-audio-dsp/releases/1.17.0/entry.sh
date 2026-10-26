#!/bin/sh
unset LD_PRELOAD
set -eu
sha256sum -c SHA256SUMS >/dev/null
echo "DSP1.17.0; Host211 required; optional same-family SoXR VHQ resampling; no resident process"
