/* Emulation-only: static entry for stock BusyBox symlinks inside the private
 * computer root. Keeps shell/file helper processes runnable without binfmt. */
#include <stdlib.h>
#include <unistd.h>
int main(int argc,char **argv){
 (void)argc;unsetenv("LD_PRELOAD");unsetenv("LD_LIBRARY_PATH");
 execv("/emu/host-busybox",argv);return 127;
}
