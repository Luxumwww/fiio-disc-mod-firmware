#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include "tf_mount_guard.h"
int main(void){
 assert(tf_root_cleanup("rm -rf /tmp/sdcard"));assert(tf_root_cleanup("rm -rf /tmp/sdcard/ "));
 assert(!tf_root_cleanup("rm -rf /tmp/sdcard/log"));assert(!tf_root_cleanup("rm -rf /tmp/sdcard-other"));assert(!tf_root_cleanup("rm -rf /tmp/sdcard; echo x"));
 assert(tf_mounted_text("/dev/root / squashfs ro 0 0\n/dev/mmcblk0p1 /tmp/sdcard exfat rw 0 0\n"));
 assert(!tf_mounted_text("/dev/root / squashfs ro 0 0\n/dev/mmcblk0p1 /tmp/sdcard-other exfat rw 0 0\n"));
 assert(!tf_mounted_text("/dev/root / squashfs ro 0 0\n"));puts("PASS mounted card root cleanup guard; unmounted/log/other paths unchanged");
}
