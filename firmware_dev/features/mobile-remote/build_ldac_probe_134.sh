set -eu
cd '/mnt/d/AI workflow/codex/disc/firmware_dev/features/mobile-remote'
clang --target=mipsel-linux-gnu -march=mips32r2 -mabi=32 -mhard-float -mfp64 -mnan=2008 -O2 -Wall -Wextra -Werror -fPIC -shared -nostdlib -fuse-ld=lld -L/root/disc-official-component-base-20261004-r1/base/lib ../../maintenance/probe_cpu_attribution.c -Wl,-l:libdl.so.2,-l:libc.so.6,--no-undefined,-z,noexecstack -o reports/host134/probe-cpu.so
