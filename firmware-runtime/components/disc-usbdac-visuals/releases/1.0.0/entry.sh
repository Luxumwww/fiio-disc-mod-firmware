#!/bin/sh
unset LD_PRELOAD
set -eu
sha256sum -c SHA256SUMS >/dev/null
echo "USB DAC visual host feature; requires shared host BT1.3.34/UI1.10.34 and XY/polar/spectrum providers. No daemon or audio device."
