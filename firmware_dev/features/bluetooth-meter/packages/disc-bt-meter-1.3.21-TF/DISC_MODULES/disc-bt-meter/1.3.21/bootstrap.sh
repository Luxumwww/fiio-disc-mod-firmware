#!/bin/sh
# One-shot integration; no resident module process and no device reboot.
unset LD_PRELOAD
set -eu
umask 077
root=/usr/data/disc-runtime
ram=/tmp/disc-runtime
release=$(pwd -P)
case "$release" in "$root/components/disc-bt-meter/releases/"*|/tmp/sdcard/DISC_MODULES/disc-bt-meter/*) :;;*) exit 2;;esac
test "$(cat "$root/service/API")" = 6
test "$(sha256sum /usr/bin/mq_ui | cut -d' ' -f1)" = f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226
sha256sum -c SHA256SUMS >/dev/null
loader=$(sha256sum /usr/data/lib/librt.so.1 | cut -d' ' -f1)
expected_loader=$(sha256sum bridge/librt-ui-loader.so.1 | cut -d' ' -f1)
expected_menu=$(sha256sum bridge/disc-settings.so | cut -d' ' -f1)
menu='';test ! -f /usr/data/disc-settings/module-ui.so || menu=$(sha256sum /usr/data/disc-settings/module-ui.so | cut -d' ' -f1)
framework=$(sha256sum "$root/service/runtime.sh" | cut -d' ' -f1)
expected_framework=$(sha256sum framework/runtime.sh | cut -d' ' -f1)
if [ "$loader:$menu:$framework" = "$expected_loader:$expected_menu:$expected_framework" ];then exit 0;fi
case "$loader" in 1ccdf3b3ceb637bcc2ca6a72a8e37bf8d5008c6c532cb160f80e179c368f08fb|"$expected_loader") :;;*) echo 'unknown loader preserved' >&2;exit 3;;esac
case "$menu" in ''|1e459748b05aa2ca971f59a837d6071c57234fc4c9aec669432dd4bfafa7fb15|9cd1ee8b372b7b3c397b92097473ef1efbe4ad6c10fbb53bba0477aec3788570|16737a1566d9babe3e8cfd8058189c03693d75a878fd1ed2c39ba4279010cdee|3f08f894387055fa2349a550e742cd61fb1a03ec5320b6508ca714e98d74e67e|9399a1d30a35d20189979070eab10b4c8437d9ee631ad5eb6ca6a89c45a86fd9|077e1df5a2fb8be6dd7f8798b0f4c08b6e58a0f7291520b1af2336e753d43cec|cc8eca8a83bdb76615cb681e6dcfd5035484be3eca6ec5687a8a048bbe7fa80d|404725d4645eaa07c51e5a24516050ee610b434351f85d6d82610224ace616af|e4918cb80eede5999729e3d9d86cf17c429209d440d162e0ffbbcabf9ab08fe2|f5779ba6ba57fb0cb910ad21fb615f55e67af0e5ac8fcd36ce27c01d9e87115c|e784fcde06c749656c0cdad9c296986ded2c78aec6e365b65c3ac4943f886244|93f10d990a077020c60bad37bc8c44347ae8a02411fd63a31159d8d64f994658|df91dc11773548e14681174a4841ba9cc86ac9065df8430650e4f80e90355919|36b3aa19f91849b892c7425e969ab2cbfc48692b7e4a6cdcefe37bcc3c759f18|46136f5c8415b5e20f015d3eb5a815f106ca3e53a076ae497bf21761f2c986fb|b0355f1bc486855c367e936f4fe6435774a8bb1be5609f3639da020d8099f482|5701f7a9c32845b07f3ee27a9180411bd5b9f40842c4b6ce425ee0d670bb2301|1c22e316d11c8e841baa42ebd9b4061865fea5e06508f7c3d50501bdccef96cb|7a05ade81e4f43c1a72ced1c4cd9447b041f7e01622672fdaa35f8e0b4ce3650|6225a249bdcfa01940a8b95bec88e5747b8f5b764fedb3f5a8360c671fd7e9ba|a1e7eacf1c59e6a6588982c634b406433c55ae400db8821363d89c89915c57a1|8b9a0c5aaf0dd02d609e53fd851794de59d7b7fa1e585cfeaf5cef32cecc50d1|1a5acfa669c9b06faff170b24f45b044628e8d67c6837badc5fcdbec9df042ab|fcfa63d8a4ea9ad8b453f0534071e5b3cab71057f883d49c4744657037b8990e|6fd9d92b79a91d86b1a560e1efad43b43ce14367dfdd77bcf7032cf2b49a2f41|7df1c39fc968ca9b97e0cf2b0dfd8109a19205ae516021ec37ffb458e1e279aa|55da9db23e6b008d56609d16133ba4beb60b82a0e824a516ab5673bfc2ba5e47|"$expected_menu") :;;*) echo 'unknown module UI preserved' >&2;exit 3;;esac
case "$framework" in 64155d43f21618bc5306fafec0a2420de6e65f50537774c07842895c95ee246c|7f235c3b219694b5e2e1483b3d8cb57515a66604382e6e48a35dd0fd74249c90|8353f9aaf10fbba74fc7df781329125197a8436952b08a09102d10c5c468564b|00ab7aeb3688bd054173c54ae75789bc09c6f96476a0c49d6b3714f2d86ab9a8|e24673565583934d7070aa090f7efffe5ac22c7e26c65304f0a5c60f3ceb31e6|efc9a60a5d383be0973311397a5e447fdc602a918482d908dcf5dd4788df6fbe|247fdf2aa612c0ff6975d535d56e6211c40b80c8b3f430865ad9c39f91713678|247fdf2aa612c0ff6975d535d56e6211c40b80c8b3f430865ad9c39f91713678|"$expected_framework") :;;*) echo 'unknown runtime preserved' >&2;exit 3;;esac
test ! -d "$ram/personal-lock"
mkdir "$ram/bt-install.lock" || exit 4
cleanup(){ rmdir "$ram/bt-install.lock" 2>/dev/null || true; }
trap cleanup EXIT
trap 'exit 130' INT TERM
stamp=$(cut -d. -f1 /proc/uptime)-$$
backup=$root/bt-before-$stamp
stage=$root/incoming/bt-service-$stamp
test ! -e "$backup" && test ! -e "$stage"
mkdir "$backup"
cp /usr/data/lib/librt.so.1 "$backup/librt.so.1"
cp /usr/data/disc-settings.so "$backup/disc-settings.so"
cp /usr/data/disc-settings/VERSION "$backup/menu-VERSION"
test ! -f /usr/data/disc-settings/module-ui.so || cp /usr/data/disc-settings/module-ui.so "$backup/module-ui.so"
cp -a "$root/service" "$stage"
for f in runtime.sh runtime-modules.sh module-state.sh mode-policy.sh;do cp "framework/$f" "$stage/$f.new";chmod 755 "$stage/$f.new";mv "$stage/$f.new" "$stage/$f";done
(
 cd "$stage"
 find . -type f ! -name SHA256SUMS ! -name SHA256SUMS.new | sort | while read -r f;do sha256sum "$f";done > SHA256SUMS.new
 mv SHA256SUMS.new SHA256SUMS
 sha256sum -c SHA256SUMS >/dev/null
 sh -n runtime.sh;sh -n runtime-modules.sh;sh -n module-state.sh;sh -n mode-policy.sh
)
mkdir -p "$root/module-data/disc-bt-meter"
cp bridge/disc-settings.so /usr/data/disc-settings/module-ui.so.bt-new
cp bridge/librt-ui-loader.so.1 /usr/data/lib/librt.so.1.bt-new
test "$(sha256sum /usr/data/disc-settings/module-ui.so.bt-new | cut -d' ' -f1)" = "$expected_menu"
test "$(sha256sum /usr/data/lib/librt.so.1.bt-new | cut -d' ' -f1)" = "$expected_loader"
chmod 644 /usr/data/disc-settings/module-ui.so.bt-new /usr/data/lib/librt.so.1.bt-new
# TERM can leave the old shell's empty RAM lock. Remove it only after the
# verified runtime process exits, and preserve all configuration/modules.
p=$(cat "$ram/manager.pid")
case "$p" in ''|*[!0-9]*) exit 5;;esac
grep -q '/usr/data/disc-runtime/service/runtime.sh' /proc/$p/cmdline || exit 5
kill -TERM "$p"
n=0;while [ -d /proc/$p ] && [ "$n" -lt 12 ];do sleep 1;n=$((n+1));done
test ! -d /proc/$p
test ! -d "$ram/lock" || rmdir "$ram/lock"
mv "$root/service" "$backup/service"
mv "$stage" "$root/service"
# Dependency first, then loader. Immutable firmware deliberately preserves
# a different, user-owned loader on ordinary boots.
mv /usr/data/disc-settings/module-ui.so.bt-new /usr/data/disc-settings/module-ui.so
mv /usr/data/lib/librt.so.1.bt-new /usr/data/lib/librt.so.1
printf '1.10.21\n' > /usr/data/disc-settings/VERSION
printf '%s\n' "$backup" > "$ram/bt-install-backup"
sync
nohup setsid "$root/service/runtime.sh" > /tmp/disc-runtime-launch.log 2>&1 </dev/null &
n=0;ready=0
while [ "$n" -lt 20 ];do
 sleep 1;n=$((n+1));newp=$(cat "$ram/manager.pid")
 if [ "$newp" != "$p" ] && [ -d /proc/$newp ] && [ "$(cat "$ram/bt-module.path" 2>/dev/null)" = "$release/libdisc-bt-meter.so" ];then ready=1;break;fi
done
test "$ready" = 1
if [ "$loader:$menu" != "$expected_loader:$expected_menu" ];then
 ui=$(pidof mq_ui);player=$(pidof mq_player)
 test "$(readlink /proc/$ui/exe)" = /usr/bin/mq_ui
 test "$(readlink /proc/$player/exe)" = /usr/bin/mq_player
 # Stock UI and player share queues. Let the original supervisor reload
 # both normally; the previous attempt to keep the player was insufficient.
 kill -TERM "$ui"
fi
echo 'Bluetooth module integrated; stock supervisor reload requested; backups preserved'
