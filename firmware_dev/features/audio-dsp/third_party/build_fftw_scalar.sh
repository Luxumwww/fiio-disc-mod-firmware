#!/bin/sh
set -eu
work=/tmp/disc-fftw-scalar-src/fftw-3.3.10
destination='/mnt/d/AI workflow/codex/disc/firmware_dev/features/audio-dsp/third_party/fftw-scalar-mips'
cd "$work"
export CC='clang --target=mipsel-linux-gnu'
export AR=ar RANLIB=ranlib
mkdir -p /tmp/disc-fftw-src/compat/bits
python3 - <<'PY'
from pathlib import Path
s=Path('/usr/include/bits/alltypes.h').read_text()
s=s.replace('#define _Addr long','#define _Addr int').replace('#define _Reg long','#define _Reg int').replace('#define _Int64 long','#define _Int64 long long')
s=s.replace('0x7fffffffffffffffL','0x7fffffffL').replace('#define __USE_TIME_BITS64 1','#define __USE_TIME_BITS64 0')
s=s.replace('typedef _Int64 time_t;','typedef long time_t;').replace('typedef _Int64 suseconds_t;','typedef long suseconds_t;')
Path('/tmp/disc-fftw-src/compat/bits/alltypes.h').write_text(s)
PY
export CFLAGS='-O3 -fPIC -I/tmp/disc-fftw-src/compat -march=mips32r5 -mmsa -mabi=32 -mhard-float -mfp64 -mnan=2008 -ffp-contract=fast -fno-math-errno'
export ac_cv_func_gettimeofday=no ac_cv_func_clock_gettime=no
export LDFLAGS='-nostdlib -fuse-ld=lld -L/root/disc-official-component-base-20261004-r1/base/lib -Wl,--entry,main,-z,noexecstack'
export LIBS='-Wl,-l:libc.so.6,-l:libm.so.6'
./configure --host=mipsel-linux-gnu --build=x86_64-pc-linux-musl --prefix="$destination" --enable-float --disable-generic-simd128 --with-pic --disable-shared --enable-static --disable-fortran --disable-doc --disable-threads --disable-openmp --disable-mpi --disable-dependency-tracking > configure-disc.log 2>&1 || { tail -45 configure-disc.log; exit 1; }
for directory in kernel dft rdft reodft simd-support api; do
 make -C "$directory" -j4 > "build-$directory-disc.log" 2>&1 || { tail -55 "build-$directory-disc.log"; exit 1; }
done
make -j4 libfftw3f.la > build-library-disc.log 2>&1 || { tail -55 build-library-disc.log; exit 1; }
mkdir -p "$destination/lib" "$destination/include"
cp .libs/libfftw3f.a "$destination/lib/"
cp api/fftw3.h "$destination/include/"
cp COPYING config.h configure-disc.log build-*-disc.log "$destination/"
echo 'FFTW single-precision static MIPS MSA library built'
