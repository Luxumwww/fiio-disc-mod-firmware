set -eu
cd '/mnt/d/AI workflow/codex/disc/firmware_dev/features/mobile-remote'
python3 test_usb_volume_132.py
base=/root/disc-official-component-base-20261004-r1/base
clang --target=mipsel-linux-gnu -march=mips32r2 -mabi=32 -mhard-float -mfp64 -mnan=2008 -O2 -Wall -Wextra -Werror -fPIC -shared -nostdlib -fuse-ld=lld -L"$base/lib" test_stock_usb_gain_132.c -Wl,-l:libc.so.6,--no-undefined,-z,noexecstack -o reports/usb-volume-132/stock-gain-test.so
cp reports/usb-volume-132/stock-gain-test.so /tmp/disc-stock-gain-132.so
qemu-mipsel -cpu P5600 -L "$base" -E LD_LIBRARY_PATH="$base/lib:$base/usr/lib" -E LD_PRELOAD=/tmp/disc-stock-gain-132.so "$base/usr/bin/mq_player" > reports/usb-volume-132/stock-gain-test.txt 2>&1
cat reports/usb-volume-132/stock-gain-test.txt
python3 ../bluetooth-meter/build.py > reports/usb-volume-132/build.log 2>&1
cat ../bluetooth-meter/build/build.json
