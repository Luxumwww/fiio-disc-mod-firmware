#define main legacy_test_main
#include "test_dop_compat.c"
#undef main
int main(void){
 unsigned p=0x810000;serial_feature=1;disabled=1;mode=1;
 PC_WORD(0x83a6bc)=0x4e9534;PC_WORD(0x83a704)=0x4e94f4;
 PC_WORD(PLAYER_IOCTL_GOT)=(unsigned)(unsigned long)fake_ioctl;
 dop_install();assert(!dop_original_configure&&!dop_real_ioctl);
 disabled=0;mode=4;clock_sec++;PC_WORD(0x83dfc4)=p;
 PC_BYTE(0x83a75a)=6;PC_WORD(0x839a74)=1;
 PC_WORD(p+0xc)=PC_WORD(p+0x1c)=352800;PC_WORD(p+0x10)=PC_WORD(p+0x20)=2;
 PC_WORD(p+0x14)=PC_WORD(p+0x24)=32;PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;PC_WORD(p+0x98)=2;
 PC_WORD(0x83a7d0)=8;PC_WORD(0x83a7cc)=9;
 dop_install();assert(dop_serial_profile&&dop_stock_profile&&dop_enabled&&dop_eq_ready);
 assert(PC_WORD(PLAYER_IOCTL_GOT)==(unsigned)(unsigned long)dop_serial_ioctl);
 native_noop=1;native_ioctl_result=1;PC_BYTE(0x899060)=1;PC_WORD(0x899084)=352800;
 for(unsigned c=0;c<2;c++){busregs[c][2]=23;busregs[c][4]=47;busregs[c][8]=0xa2;}
 assert(!dop_configure(352800,32,1));
 assert(mute_calls==1&&unmute_calls==1&&dop_serial_applied==1&&dop_frame_repairs==2&&!dop_serial_failed);
 assert(busregs[0][2]==31&&busregs[1][4]==63&&!rate_closes&&!rate_opens);
 unsigned writes=dop_bus_writes;
 for(unsigned i=0;i<20;i++){assert(!dop_serial_ioctl(8,0x20004d28,0));assert(!dop_serial_ioctl(9,0x20004d2b,0));}
 assert(dop_health_avoided==40&&native_health==0&&dop_bus_writes==writes);
 assert(!dop_configure(352800,32,1)&&mute_calls==1&&unmute_calls==1);
 unsigned packet[4]={0x05aabb00,0x05ddeeff,0xfa123400,0xfa567800},copy[4];memcpy(copy,packet,sizeof copy);
 assert(!dop_eq_filter(packet,sizeof packet,352800)&&!memcmp(packet,copy,sizeof copy)&&!dop_gain_filter());
 PC_WORD(p+0xc)=PC_WORD(p+0x1c)=176400;PC_WORD(0x899084)=176400;
 assert(!dop_configure(176400,32,1)&&busregs[0][0]==5&&busregs[1][1]==0x50);
 assert(dop_serial_applied==2&&mute_calls==2&&unmute_calls==2);
 busregs[1][6]=0x99;unsigned unmuted=unmute_calls;
 assert(dop_serial_ioctl(8,0x20004d2e,0)==1&&unmute_calls==unmuted+1&&dop_serial_failed==1);
 assert(dop_serial_ioctl(9,0x20004d2b,0)==1&&dop_serial_health_forwarded==1&&native_health==1);
 busregs[1][6]=0x0c;
 mode=1;assert(dop_serial_ioctl(8,0x20004d28,0)==1&&native_health==2);
 mode=2;assert(dop_serial_ioctl(8,0x20004d28,0)==1&&native_health==3);
 mode=4;route=3;assert(dop_serial_ioctl(8,0x20004d28,0)==1&&native_health==4);route=6;
 PC_WORD(p+0x2c)=1;assert(dop_serial_ioctl(8,0x20004d28,0)==1&&native_health==5);
 PC_WORD(p+0x2c)=2;disabled=1;clock_sec++;dop_gate_refresh();
 assert(dop_serial_ioctl(8,0x20004d28,0)==1&&native_health==6);
 assert(!rate_closes&&!rate_opens&&!dop_quiet_bypasses);
 puts("PASS: minimal serial profile64/128; two-DAC48-to64 frame repair; valid DSD health skips stock wrong-rate reset; positive ioctl success accepted; unknown state forwards and unmute never latches; PCM/local/BT/external/disabled unchanged; packet bytes preserved; no PCM reopen or silence suppression");
}
