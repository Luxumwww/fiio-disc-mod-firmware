#!/bin/sh
set -ex
unset LD_PRELOAD
export DISC_PERSONAL_TEST=1
export DISC_PERSONAL_DATA=/usr/data/disc-personal-fixture-1791184675
export DISC_PERSONAL_TF=/tmp/sdcard/DISC_PERSONAL_TEST_1791184675
export DISC_PERSONAL_RAM=/tmp/disc-personal-fixture-1791184675
export DISC_PERSONAL_SERVICE=/usr/data/disc-runtime/service
test ! -e "$DISC_PERSONAL_DATA";test ! -e "$DISC_PERSONAL_TF";test ! -e "$DISC_PERSONAL_RAM"
mkdir -p "$DISC_PERSONAL_DATA/fiio/db" "$DISC_PERSONAL_TF" "$DISC_PERSONAL_RAM"
mkdir -p "$DISC_PERSONAL_DATA/storebluetooth/lib/bluetooth/40:D9:5A:E3:8E:35"
echo synthetic-key > "$DISC_PERSONAL_DATA/storebluetooth/lib/bluetooth/40:D9:5A:E3:8E:35/info"
printf 'synthetic-board\n' > "$DISC_PERSONAL_DATA/macaddr.txt"
cat > "$DISC_PERSONAL_DATA/wpa_supplicant.conf" <<'EOF'
ctrl_interface=/tmp/wpa_supplicant
network={
 ssid="DISC-LAB"
 psk="synthetic-test-password"
}
network={
 ssid="DISC-LAB-OPEN"
 key_mgmt=NONE
}
EOF
sqlite3 "$DISC_PERSONAL_DATA/fiio/db/sysconfig.db" 'CREATE TABLE test(value TEXT); INSERT INTO test VALUES("original");'
s=$DISC_PERSONAL_SERVICE
for f in "$s"/*.sh;do sh -n "$f";done
"$s/personal-state.sh"
test "$(wc -l < "$DISC_PERSONAL_RAM/wifi-list")" = 2
! grep -q 'synthetic-test-password' "$DISC_PERSONAL_RAM/wifi-list"
key=$(sed -n '1s/|.*//p' "$DISC_PERSONAL_RAM/wifi-list")
"$s/personal-control.sh" secret "$key"
test "$(sed -n '2p' "$DISC_PERSONAL_RAM/wifi-secret")" = synthetic-test-password
"$s/personal-control.sh" forget "$key"
test "$(wc -l < "$DISC_PERSONAL_RAM/wifi-list")" = 1
! grep -q 'DISC-LAB"' "$DISC_PERSONAL_DATA/wpa_supplicant.conf"
"$s/personal-control.sh" backup
id=$(head -n 1 "$DISC_PERSONAL_RAM/backup-list")
"$s/personal-backup.sh" validate "$DISC_PERSONAL_TF/DISC_DATA_BACKUP/$id"
"$s/personal-control.sh" backup
test "$(wc -l < "$DISC_PERSONAL_RAM/backup-list")" = 2
sqlite3 "$DISC_PERSONAL_DATA/fiio/db/sysconfig.db" 'UPDATE test SET value="changed";'
cat > "$DISC_PERSONAL_DATA/wpa_supplicant.conf" <<'EOF'
ctrl_interface=/tmp/wpa_supplicant
network={
 ssid="DISC-LAB-NEXT"
 psk="another-fixture-password"
}
EOF
"$s/personal-state.sh"
test "$(wc -l < "$DISC_PERSONAL_RAM/wifi-list")" = 2
key=$(sed -n '1s/|.*//p' "$DISC_PERSONAL_RAM/wifi-list")
"$s/personal-control.sh" connect "$key"
! grep -q 'DISC-LAB-NEXT' "$DISC_PERSONAL_DATA/wpa_supplicant.conf"
test "$(wc -l < "$DISC_PERSONAL_RAM/wifi-list")" = 2
"$s/personal-control.sh" restore "$id"
test "$(sqlite3 "$DISC_PERSONAL_DATA/fiio/db/sysconfig.db" 'SELECT value FROM test;')" = original
test "$(cat "$DISC_PERSONAL_DATA/storebluetooth/lib/bluetooth/40:D9:5A:E3:8E:35/info")" = synthetic-key
test "$(wc -l < "$DISC_PERSONAL_RAM/backup-list")" = 3
test "$(wc -l < "$DISC_PERSONAL_RAM/wifi-list")" = 1
"$s/personal-control.sh" backup-delete "$id"
test ! -d "$DISC_PERSONAL_TF/DISC_DATA_BACKUP/$id"
test "$(wc -l < "$DISC_PERSONAL_RAM/backup-list")" = 2
test "$(ls -l "$DISC_PERSONAL_DATA/wpa_supplicant.conf" | awk '{print $1}')" = '-rw-------'
printf 'native_fixture_passed\n'
