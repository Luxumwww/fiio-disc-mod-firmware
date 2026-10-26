#define main legacy_test_main
#include "test_dop_compat.c"
#undef main
int main(void){
 unsigned p=0x810000;stock_feature=1;mode=4;
 PC_WORD(0x83a6bc)=0x4e9534;PC_WORD(0x83a704)=0x4e94f4;
 PC_WORD(PLAYER_IOCTL_GOT)=(unsigned)(unsigned long)fake_ioctl;
 PC_WORD(0x83dfc4)=p;PC_BYTE(0x83a75a)=6;PC_WORD(0x839a74)=1;
 PC_WORD(p+0xc)=PC_WORD(p+0x1c)=352800;PC_WORD(p+0x10)=PC_WORD(p+0x20)=2;
 PC_WORD(p+0x14)=PC_WORD(p+0x24)=32;PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;PC_WORD(p+0x98)=2;
 PC_WORD(0x83a7d0)=8;PC_WORD(0x83a7cc)=9;PC_BYTE(0x899060)=1;PC_WORD(0x899084)=352800;
 native_noop=1;native_ioctl_result=1;
 for(unsigned c=0;c<2;c++){busregs[c][2]=23;busregs[c][4]=47;busregs[c][8]=0xa2;}
 dop_install();assert(dop_stock_profile&&!dop_serial_profile&&!dop_real_ioctl);
 serial_feature=1;clock_sec++;dop_tick();
 assert(dop_serial_profile&&PC_WORD(PLAYER_IOCTL_GOT)==(unsigned)(unsigned long)dop_serial_ioctl);
 assert(dop_serial_applied==1&&dop_frame_repairs==2&&mute_calls==1&&unmute_calls==1);
 dop_tick();assert(dop_serial_applied==1&&mute_calls==1);
 PC_WORD(PLAYER_IOCTL_GOT)=1234;assert(!dop_serial_bind()&&PC_WORD(PLAYER_IOCTL_GOT)==1234);
 puts("PASS: existing stock profile switches to serial profile on next control tick; active same-rate stream repaired once; foreign ioctl hook preserved");
}
