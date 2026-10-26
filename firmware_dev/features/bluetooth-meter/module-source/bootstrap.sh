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
# The current host bridge is an explicitly reviewed base for the next host-only update.
[ "$menu" = 005dc90d15328aef1ac86cd0145e4ee6de8ef84384f8e92d18c087a262d333a3 ] && menu=''
framework=$(sha256sum "$root/service/runtime.sh" | cut -d' ' -f1)
expected_framework=$(sha256sum framework/runtime.sh | cut -d' ' -f1)
# Diagnostic host-only update: verified bridge replacement, no new backup archive.
if [ "$loader:$framework" = "$expected_loader:$expected_framework" ] && [ "$menu" = fb67ad8010136bba3226c5b0a40bc846370d6e777b51a3df0d5dcf35e55da4a7 ];then
 ui=$(pidof mq_ui);player=$(pidof mq_player)
 test "$(readlink /proc/$ui/exe)" = /usr/bin/mq_ui
 test "$(readlink /proc/$player/exe)" = /usr/bin/mq_player
 test ! -e "$ram/ram-dop-start"
 cp bridge/disc-settings.so /usr/data/disc-settings/module-ui.so.ram-next
 test "$(sha256sum /usr/data/disc-settings/module-ui.so.ram-next | cut -d' ' -f1)" = "$expected_menu"
 chmod 644 /usr/data/disc-settings/module-ui.so.ram-next
 mv /usr/data/disc-settings/module-ui.so.ram-next /usr/data/disc-settings/module-ui.so
 printf '1.10.68\n' > /usr/data/disc-settings/VERSION
 sync
 kill -TERM "$ui"
 echo 'RAM diagnostic bridge installed; normal UI reload requested'
 exit 0
fi
if [ "$loader:$menu:$framework" = "$expected_loader:$expected_menu:$expected_framework" ];then
 # An unchanged UI does not imply the player preload is current. Audio-only
 # releases must reload the selected host, without copying another backup.
 selected=$(readlink "$root/components/disc-bt-meter/current")
 applied=$(cat "$ram/integrated-disc-bt-meter" 2>/dev/null || true)
 if [ "$applied" = "$selected" ];then exit 0;fi
 test "$(cat "$ram/bt-module.path" 2>/dev/null)" = "$release/libdisc-bt-meter.so"
 ui=$(pidof mq_ui);player=$(pidof mq_player)
 test "$(readlink /proc/$ui/exe)" = /usr/bin/mq_ui
 test "$(readlink /proc/$player/exe)" = /usr/bin/mq_player
 printf '1.10.68\n' > /usr/data/disc-settings/VERSION.next
 mv /usr/data/disc-settings/VERSION.next /usr/data/disc-settings/VERSION
 kill -TERM "$ui"
 echo 'Audio host reload requested; identical UI retained'
 exit 0
fi
case "$loader" in 1ccdf3b3ceb637bcc2ca6a72a8e37bf8d5008c6c532cb160f80e179c368f08fb|"$expected_loader") :;;*) echo 'unknown loader preserved' >&2;exit 3;;esac
case "$menu" in 27f08db60260dbeb8fe24e862b00f8c4f52b494112f1341847f8117e31d4540a|1074d42d926d24321722822c429e08fa634b0a2658cdbd5f3a7a29e8e4c3a26b|a2c7932bd5addf2d0ba3f415a1cb36510ae9de4738e75de03e1e9359f9520eb8|c6eaed952ea048ba995ededd1389da26ebfd9d1aa7054ee0c4e9f1361c4011b4|7ff021ce4946fe719c7e011f856f8eda92f68c424ca983253a6d30e9dc521b5e|d4c4c8d317403b82b7a858287a717c4eef1e92e0936be67bde24ab1ab9e1a891|2c6edf0df9c68d7c92019d616332ba373d20dda552e196d32ebfee84aa3e35a2|a82493e8187b0cf99d51cabd9b8070514ae19e2afba2a66eb215710c72327154|282374168b250505be3ea3e87c630aec27a36acdbb1574d928853017ec4adfe9|a67ff4c47cde97dbdcd5c53b3b43a20312698f1bb7ad0621ac1acdd95471c12b|''|1e459748b05aa2ca971f59a837d6071c57234fc4c9aec669432dd4bfafa7fb15|9cd1ee8b372b7b3c397b92097473ef1efbe4ad6c10fbb53bba0477aec3788570|16737a1566d9babe3e8cfd8058189c03693d75a878fd1ed2c39ba4279010cdee|3f08f894387055fa2349a550e742cd61fb1a03ec5320b6508ca714e98d74e67e|9399a1d30a35d20189979070eab10b4c8437d9ee631ad5eb6ca6a89c45a86fd9|077e1df5a2fb8be6dd7f8798b0f4c08b6e58a0f7291520b1af2336e753d43cec|cc8eca8a83bdb76615cb681e6dcfd5035484be3eca6ec5687a8a048bbe7fa80d|404725d4645eaa07c51e5a24516050ee610b434351f85d6d82610224ace616af|e4918cb80eede5999729e3d9d86cf17c429209d440d162e0ffbbcabf9ab08fe2|f5779ba6ba57fb0cb910ad21fb615f55e67af0e5ac8fcd36ce27c01d9e87115c|e784fcde06c749656c0cdad9c296986ded2c78aec6e365b65c3ac4943f886244|93f10d990a077020c60bad37bc8c44347ae8a02411fd63a31159d8d64f994658|df91dc11773548e14681174a4841ba9cc86ac9065df8430650e4f80e90355919|36b3aa19f91849b892c7425e969ab2cbfc48692b7e4a6cdcefe37bcc3c759f18|46136f5c8415b5e20f015d3eb5a815f106ca3e53a076ae497bf21761f2c986fb|b0355f1bc486855c367e936f4fe6435774a8bb1be5609f3639da020d8099f482|5701f7a9c32845b07f3ee27a9180411bd5b9f40842c4b6ce425ee0d670bb2301|1c22e316d11c8e841baa42ebd9b4061865fea5e06508f7c3d50501bdccef96cb|7a05ade81e4f43c1a72ced1c4cd9447b041f7e01622672fdaa35f8e0b4ce3650|6225a249bdcfa01940a8b95bec88e5747b8f5b764fedb3f5a8360c671fd7e9ba|a1e7eacf1c59e6a6588982c634b406433c55ae400db8821363d89c89915c57a1|8b9a0c5aaf0dd02d609e53fd851794de59d7b7fa1e585cfeaf5cef32cecc50d1|1a5acfa669c9b06faff170b24f45b044628e8d67c6837badc5fcdbec9df042ab|fcfa63d8a4ea9ad8b453f0534071e5b3cab71057f883d49c4744657037b8990e|6fd9d92b79a91d86b1a560e1efad43b43ce14367dfdd77bcf7032cf2b49a2f41|7df1c39fc968ca9b97e0cf2b0dfd8109a19205ae516021ec37ffb458e1e279aa|55da9db23e6b008d56609d16133ba4beb60b82a0e824a516ab5673bfc2ba5e47|d273080c56e9e814b65803d857b0081c02357cb32854f300c0ca595064ffe733|4a68843b3280228fc76bf36f58a456f8620ad1d9ad05a0fa82ef99c7381388bd|2282e1e772c7ca12159526456b35702d4a674a10ac9fe02c06dcc92105abf113|2d9222b84636ab9028af220fd7800de6634d32d17988302901d17db6ec32dffc|d22c77f3c00176a01ced7e12f49ff07c363bce3eb86bf1b8c8d14b44ac174763|20fe696616f464887d03adebc062ce0ae99f699e127e32ffecef6916ea960e5b|3a0751538d5fdca40d9bd9a488da9fb164a90dc9610e94df98cf35295bde12cb|c052333087797fac9da85154e99cd8a9522cfc5cfce6a3c0d0fb3b823cacab06|f24d5bc6ffa6719fede0ba0bf513a193e22fc461483770412e552c6d34e3eaa8|2794c97a3bf454b2969749db4de56a7268582538ad80d4003062625c2e567782|103c1cb6e32cfd6266d06eebcc099f8171b6ba7af0b07ce8a8431bab8832b5a7|a88fe66831f7c9718c699cc1795809be1c6305d50ebb90ff36ab6fb64db9e94c|488fd62fd10eeb9bf43dcdf505298a221ca98ef5d74252e83bcf9a43ad2bdbfb|4b124dd219788d3dacf54e29d288a3c49b226888d995294fc9042a7829bdc290|71c356621ba7dd71b607cf71a7c3ad7c0e292ba542be3011ac66eae4d66065da|e24aceabc76883663addb2ee5da21ea3e0940118fbb776a7bdbac612ce7f7c4f|91d5d824ad018dc55ac47aca7a2700d042da844b8480532457654a27fa1facc2|d951b9a7bcd26906953b8e45579ce6002e245e023542cf04c0057598b35eae33|aedcbb8e64e803b4139d5a027d5e5449d987b27aba709421c9f30bd420a566f6|31f7406efafd80c8270ad78935e95e317a363bfe5a147162f8974678d93f2d89|56a879402e23b6aa09f575094838d302911b71f95f42b90e93d9bf1873c0b01e|b733470e901385248b71856d1402c2945cb4ebcd79d0bb0ac772f219ed37024e|9453d51469cf8e9d02edc7825247d6413f4bb64e0720ce610ca84c7c646cd824|34f6289b264c69f80d173556c8cee590815f041d55d242f04ac8c7c6c3961200|75057bc65277965d2562f388b654d08eb7eda62c5b0be351b4e2ecd312dcd09b|a49a20b311b16689fb829ad207bb69ce0ac9c3af049692f45058f3baa2fc2a72|8aa917428f969275bc4a5956f152f779719c3b1bf0746afff6672c51ec655d8c|"$expected_menu") :;;*) echo 'unknown module UI preserved' >&2;exit 3;;esac
case "$framework" in c48fc9e145f6a46dc1e985dc436b39bf8786fc8c06ebace78ead14b0bde84cab|2e45b2cc3e4d6c98c4e13f8e619aee7b09051b6612109b1c15c39bc0fb29165f|2de2170e34fb0ddd44d84348b325bc6d4841d7a5603c9abd932a7865d183d586|64155d43f21618bc5306fafec0a2420de6e65f50537774c07842895c95ee246c|7f235c3b219694b5e2e1483b3d8cb57515a66604382e6e48a35dd0fd74249c90|8353f9aaf10fbba74fc7df781329125197a8436952b08a09102d10c5c468564b|00ab7aeb3688bd054173c54ae75789bc09c6f96476a0c49d6b3714f2d86ab9a8|e24673565583934d7070aa090f7efffe5ac22c7e26c65304f0a5c60f3ceb31e6|efc9a60a5d383be0973311397a5e447fdc602a918482d908dcf5dd4788df6fbe|247fdf2aa612c0ff6975d535d56e6211c40b80c8b3f430865ad9c39f91713678|247fdf2aa612c0ff6975d535d56e6211c40b80c8b3f430865ad9c39f91713678|b2797845db982ece962957b1ed885268261d3815a01d5e62a708c82ebc5af053|a4813cd6a6462069da7015490cbee5400402b5ec0ecc488ffe27d10881ffc88f|"$expected_framework") :;;*) echo 'unknown runtime preserved' >&2;exit 3;;esac
usb_manager=$(sha256sum "$root/service/usb-manager.sh" | cut -d' ' -f1)
expected_usb_manager=$(sha256sum framework/usb-manager.sh | cut -d' ' -f1)
case "$usb_manager" in d33683a3e4a289cb1b1e79b703ae6d27528b17d1717510a2732e8a652227f372|"$expected_usb_manager") :;;*) echo 'unknown USB manager preserved' >&2;exit 3;;esac
debug_state=$(sha256sum "$root/service/debug-state.sh" | cut -d' ' -f1)
expected_debug_state=$(sha256sum framework/debug-state.sh | cut -d' ' -f1)
case "$debug_state" in ecb2ae237be7bc97e07883428d223bbe2ab52b832aa4a8209c6992d64d939ce3|027bc6a535cdc8422567af2a7338602b42f64a5de644a6f06bfe65a5740decb8|"$expected_debug_state") :;;*) echo 'unknown debug health preserved' >&2;exit 3;;esac
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
for f in runtime.sh runtime-modules.sh module-state.sh mode-policy.sh usb-manager.sh debug-cleanup.sh debug-state.sh wifi-catalog.sh personal-state.sh;do cp "framework/$f" "$stage/$f.new";chmod 755 "$stage/$f.new";mv "$stage/$f.new" "$stage/$f";done
if [ -f framework/runtime-links ];then cp framework/runtime-links "$stage/runtime-links";chmod 755 "$stage/runtime-links";fi
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
printf '1.10.68\n' > /usr/data/disc-settings/VERSION
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
