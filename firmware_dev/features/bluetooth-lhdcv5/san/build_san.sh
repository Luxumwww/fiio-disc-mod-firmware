#!/bin/sh
# Build the host LHDC V5 decoder + D-step harness with ASan/UBSan (gcc).
# Uses the already-generated build/decoder port; does not touch build/ artifacts.
set -e
cd "$(dirname "$0")/.."
OUT=${1:-san/out}
OBJ=$OUT/obj
rm -rf "$OBJ"
mkdir -p "$OBJ"

COMMON="-std=c11 -O1 -g -fno-omit-frame-pointer -ffp-contract=off -fPIC -D_POSIX_C_SOURCE=200809L -DLHDC_HOST_BUILD -Ibuild/decoder -I."
SAN="-fsanitize=address,undefined"

for f in build/decoder/*.c; do
  o="$OBJ/$(basename "$f" .c).o"
  gcc $COMMON $SAN -include runtime_compat.h -c "$f" -o "$o"
done
gcc $COMMON $SAN -include runtime_compat.h -c codec_compat.c -o "$OBJ/codec_compat.o"
gcc $COMMON $SAN -Wall -Wextra -include runtime_compat.h -c disc_lhdcv5.c -o "$OBJ/disc_lhdcv5.o"
gcc $COMMON $SAN -c san/fuzz_lhdcv5.c -o "$OBJ/fuzz_lhdcv5.o"
gcc $SAN "$OBJ"/*.o -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc -lm -o "$OUT/fuzz_driver"
echo "built $OUT/fuzz_driver"
