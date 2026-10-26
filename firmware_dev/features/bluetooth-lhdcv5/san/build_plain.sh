#!/bin/sh
# Plain (non-sanitizer) build of the same D-step harness, for honest RSS measurement.
set -e
cd "$(dirname "$0")/.."
OUT=${1:-san/plain}
OBJ=$OUT/obj
rm -rf "$OBJ"
mkdir -p "$OBJ"
COMMON="-std=c11 -O2 -ffp-contract=off -fPIC -D_POSIX_C_SOURCE=200809L -DLHDC_HOST_BUILD -Ibuild/decoder -I."
for f in build/decoder/*.c; do
  gcc $COMMON -include runtime_compat.h -c "$f" -o "$OBJ/$(basename "$f" .c).o"
done
gcc $COMMON -include runtime_compat.h -c codec_compat.c -o "$OBJ/codec_compat.o"
gcc $COMMON -include runtime_compat.h -c disc_lhdcv5.c -o "$OBJ/disc_lhdcv5.o"
gcc $COMMON -c san/fuzz_lhdcv5.c -o "$OBJ/fuzz_lhdcv5.o"
gcc "$OBJ"/*.o -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc -lm -o "$OUT/fuzz_driver"
echo "built $OUT/fuzz_driver"
