#define DISC_DOP_TEST 1
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define PLAYER_IOCTL_GOT 0x800024
static unsigned char memory[0x100000];
#define PC_WORD(a) (*(unsigned *)(memory+(a)-0x800000))
#define PC_BYTE(a) (memory[(a)-0x800000])
static unsigned clock_sec=10,mode=4,route=6,open_pcm=1,native_calls,fix_calls,disabled,rescue,bad_kernel,bad_tag,stock_feature,serial_feature;
static int native_ioctl_result;
static unsigned native_quiet_calls,quiet_waits;
static int quiet_wait(unsigned n){assert(n==5000);quiet_waits++;return 0;}
static int native_quiet(void *p,int bytes,int quiet){(void)p;(void)quiet;native_quiet_calls++;return bytes;}
static unsigned native_eq_calls,native_gain_calls,rate_closes,rate_opens,fail_carrier,last_native_rate;
static unsigned mute_calls,unmute_calls,native_noop;
static void (*native_config_override)(unsigned,unsigned,unsigned);
static unsigned pc_seconds(void){return clock_sec;}
static int pc_workmode(void){return mode;}
static unsigned pc_route(void){return route;}
static int pc_pcm_open(unsigned r){return r==6&&open_pcm;}
static int fake_access(const char *p,int m){(void)m;return (strstr(p,"disabled")?disabled:rescue)?0:-1;}
static int fake_open(const char *p,int flags,...){(void)flags;return strstr(p,"version")?2:strstr(p,"0030")?3:strstr(p,"0032")?4:strstr(p,"i2c-3")?5:1;}
static int fake_read(int fd,void *buf,unsigned size){const char *s=fd==3?"cs43131\n":fd==4?"cs43131c\n":fd==2?(bad_kernel?"Linux version 4.4.94 #237 SMP":"Linux version 4.4.94 (FiiO) #236 SMP"):bad_tag?"other-feature\n":serial_feature?"usbdac-serial-abi1\n":stock_feature?"usbdac-stock-abi1\n":"usbdac-dop-abi3\n";unsigned n=strlen(s);assert(n<size);memcpy(buf,s,n);return n;}
static int fake_ioctl(int,unsigned,void *);
static int dop_ioctl(int,unsigned,void *);
static unsigned char busregs[2][9]={{7,0x54,31,0,63,0,0x0c,0x0a,0xa2},{7,0x54,31,0,63,0,0x0c,0x0a,0xa2}};static unsigned native_health;
static void *fake_dlsym(void *h,const char *s){(void)h;assert(!strcmp(s,"ioctl"));return (void *)fake_ioctl;}
static int fake_write(int f,const void *b,unsigned n){(void)f;(void)b;return n;}
static int fake_close(int f){(void)f;return 0;}
static int fake_unlink(const char *p){(void)p;return 0;}
static int original(unsigned r,unsigned b,unsigned d){last_native_rate=r;(void)b;native_calls++;if(!d)PC_BYTE(0x899060)=0;return 17;}
static void repaired(unsigned r,unsigned b,unsigned d){assert((r==352800||r==176400)&&b==32&&d==1);fix_calls++;if(native_noop&&PC_BYTE(0x899060)&&PC_WORD(0x899084)==r)return;PC_BYTE(0x899060)=1;PC_WORD(0x899084)=r;for(int i=0;i<2;i++){busregs[i][0]=7;busregs[i][1]=0x54;busregs[i][2]=23;busregs[i][4]=47;busregs[i][8]=0xf2;}assert(dop_ioctl(8,0x20004d2e,0)==0);}
static int native_eq(void *data,int bytes,int rate){assert(bytes>0&&rate);((unsigned *)data)[0]^=0xdeadbeefu;native_eq_calls++;return bytes;}
static int native_gain(void){native_gain_calls++;return 1;}
static void native_close(unsigned flags){assert(!flags);open_pcm=0;rate_closes++;}
static int native_rate(unsigned r,unsigned b,unsigned c,unsigned flags,unsigned dsd){assert(b==32&&c==2&&!flags&&!dsd);rate_opens++;if(fail_carrier&&r==352800)return -9;open_pcm=1;return 0;}
static void *native_fn(unsigned a){switch(a){case 0x826440:return quiet_wait;case 0x465718:return native_quiet;case 0x4e8858:return native_config_override?native_config_override:repaired;case 0x4e9534:return original;case 0x4e94f4:return native_gain;case 0x45c648:return native_eq;case 0x478354:return native_close;case 0x47859c:return native_rate;default:assert(!"unexpected native function");return 0;}}
#define PC_FN(a,t) ((t)native_fn(a))
#define access fake_access
#define open fake_open
#define read fake_read
#define close fake_close
#define dlsym fake_dlsym
#define write fake_write
#define unlink fake_unlink
#include "dop_compat.h"
static int fake_ioctl(int f,unsigned r,void *v){
 if(r!=0x707){if(r==0x20004d28||r==0x20004d2b)native_health++;if(r==0x20004d2d){mute_calls++;for(unsigned i=0;i<2;i++)busregs[i][8]=0xf2;}if(r==0x20004d2e){unmute_calls++;for(unsigned i=0;i<2;i++)busregs[i][8]=0xa2;}return native_ioctl_result;}
 assert(f==5);struct DopTransfer *x=v;struct DopMessage *m=x->messages;unsigned c=(m[0].address-0x30)/2;assert(c<2);
 unsigned char *b=m[0].buffer;unsigned reg=(b[0]<<16)|(b[1]<<8)|b[2];
 if(reg>=0x10000&&reg<=0x10002){assert(x->count==2);*m[1].buffer=reg==0x10000?0x43:reg==0x10001?0x13:0x10;return 2;}
 const unsigned rr[]={0x1000b,0x70004,0x40014,0x40015,0x40016,0x40017,0x40018,0x40019,0x20000};unsigned idx=9;
 for(unsigned i=0;i<9;i++)if(rr[i]==reg)idx=i;assert(idx<9);
 if(x->count==2){*m[1].buffer=busregs[c][idx];return 2;}
 assert(idx==0||idx==1||idx==2||idx==4);busregs[c][idx]=b[4];return 1;
}

int main(void){
 unsigned p=0x810000;PC_WORD(0x83a704)=0x4e94f4;PC_WORD(0x83a7d0)=8;PC_WORD(0x83a7cc)=9;PC_WORD(0x83dfc4)=p;PC_WORD(0x83a6bc)=0x4e9534;PC_BYTE(0x83a75a)=6;PC_WORD(0x839a74)=1;
 PC_WORD(p+0xc)=PC_WORD(p+0x1c)=352800;PC_WORD(p+0x10)=PC_WORD(p+0x20)=2;PC_WORD(p+0x14)=PC_WORD(p+0x24)=32;PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;PC_WORD(p+0x98)=2;
 /* Disabled at startup: no configure/gain/ioctl/EQ interception in local DSD. */
 disabled=1;mode=1;dop_install();
 assert(!dop_enabled&&!dop_original_configure&&!dop_original_gain&&!dop_real_ioctl&&!dop_eq_ready);
 assert(PC_WORD(0x83a6bc)==0x4e9534&&PC_WORD(0x83a704)==0x4e94f4&&!PC_WORD(PLAYER_IOCTL_GOT));
 for(unsigned i=0;i<100;i++)dop_tick();
 assert(!dop_original_configure&&PC_WORD(0x83a6bc)==0x4e9534&&!fix_calls);
 disabled=0;mode=4;clock_sec++;
 /* First install before the stock callback exists, then late initialization. */
 PC_WORD(0x83a6bc)=0;dop_install();assert(!dop_original_configure);PC_WORD(0x83a6bc)=0x4e9534;dop_tick();assert(dop_enabled&&dop_original_configure&&fix_calls==1&&PC_BYTE(0x899060)==1);for(int i=0;i<100;i++)dop_tick();assert(fix_calls==1);
 assert(dop_configure(352800,32,1)==0&&fix_calls==2);assert(dop_configure(352800,32,0)==17&&PC_BYTE(0x899060)==0);
 mode=2;assert(dop_configure(352800,32,1)==17);mode=4;route=3;assert(dop_configure(352800,32,1)==17);route=6;open_pcm=0;assert(dop_configure(352800,32,1)==17);open_pcm=1;
 PC_WORD(p+0x18)=1;assert(dop_configure(352800,32,1)==17);PC_WORD(p+0x18)=0;PC_WORD(p+0x2c)=1;assert(dop_configure(352800,32,1)==17);PC_WORD(p+0x2c)=2;
 assert(dop_configure(176400,32,1)==17); /* Mismatched input/output remains rejected. */assert(dop_configure(705600,32,1)==17);assert(dop_configure(352800,24,1)==17);
 disabled=1;clock_sec++;dop_tick();assert(!dop_enabled);assert(dop_configure(352800,32,1)==17);disabled=0;rescue=1;clock_sec++;dop_tick();assert(!dop_enabled);rescue=0;
 bad_kernel=1;clock_sec++;dop_tick();assert(!dop_enabled);bad_kernel=0;bad_tag=1;clock_sec++;dop_tick();assert(!dop_enabled);bad_tag=0;
 clock_sec++;dop_tick();assert(dop_enabled&&fix_calls==3);dop_tick();assert(fix_calls==3);
 PC_WORD(p+0xc)=PC_WORD(p+0x1c)=176400;assert(dop_configure(176400,32,1)==0&&fix_calls==4);
 assert(busregs[0][0]==5&&busregs[1][1]==0x50);unsigned w=dop_bus_writes;for(int i=0;i<12;i++){assert(!dop_ioctl(8,0x20004d28,0));assert(!dop_ioctl(9,0x20004d2b,0));}assert(native_health==0&&dop_bus_writes==w&&dop_health_avoided==24);busregs[0][0]=7;unsigned health_before=native_health;assert(!dop_ioctl(8,0x20004d28,0));assert(native_health==health_before&&busregs[0][0]==5);disabled=1;clock_sec++;dop_tick();dop_ioctl(8,0x20004d28,0);assert(native_health==1);
 disabled=0;clock_sec++;dop_tick();PC_WORD(p+0xc)=352800;PC_WORD(p+0x1c)=96000;PC_BYTE(0x899060)=0;PC_BYTE(0x85f428)=1;
 unsigned packet[4]={0x0569aaaa,0x05559999,0xfaeeaa11,0xfa55ee99},original_packet[4];memcpy(original_packet,packet,sizeof packet);
 assert(dop_eq_filter(packet,sizeof packet,96000)==0&&!memcmp(packet,original_packet,sizeof packet)&&!dop_gain_filter()&&!native_eq_calls&&!native_gain_calls&&PC_BYTE(0x85f428)==1);
 assert(dop_configure(96000,32,1)==0&&PC_WORD(p+0x1c)==352800&&PC_BYTE(0x899060)==1&&dop_rate_repairs==1&&rate_closes==1);
 PC_WORD(p+0x1c)=96000;native_noop=1;unsigned unmute_before=unmute_calls;
 assert(dop_configure(96000,32,1)==0&&PC_WORD(p+0x1c)==352800&&unmute_calls==unmute_before+1);native_noop=0;
 assert(!dop_quiet_filter(packet,sizeof packet,1)&&dop_quiet_bypasses==1&&!native_quiet_calls&&quiet_waits==1);assert(dop_quiet_filter(packet,sizeof packet,0)==sizeof packet&&native_quiet_calls==1);
 unsigned sample[64];for(unsigned i=0;i<64;i++)sample[i]=0x05aabb00+i;
 dop_capture_phase=1;assert(dop_eq_filter(sample,sizeof sample,352800)==0&&dop_capture_phase==2&&!memcmp(sample,dop_capture_data,sizeof sample));dop_capture_phase=0;
 assert(!dop_eq_filter(sample,300000,352800)&&dop_pre_max==300000&&dop_pre_oversize==1&&dop_pre_truncated_bytes==54240);
 PC_WORD(p+0x2c)=1;PC_WORD(p+0x58)=0;assert(dop_configure(352800,32,0)==17&&PC_WORD(p+0x1c)==96000&&last_native_rate==96000&&!dop_saved_player);
 assert(dop_quiet_filter(packet,sizeof packet,1)==sizeof packet&&native_quiet_calls==2);
 assert(dop_eq_filter(packet,sizeof packet,96000)==sizeof packet&&native_eq_calls==1&&dop_gain_filter()==1&&native_gain_calls==1&&PC_BYTE(0x85f428)==1);
 PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;fail_carrier=1;unsigned failures=dop_rate_failures;
 assert(dop_configure(96000,32,1)==-1&&open_pcm&&PC_WORD(p+0x1c)==96000&&dop_rate_failures==failures+1);
 puts("PASS: DSD64/128 carrier/speed and healthy checks; rate-capped output repaired and PCM cap restored; DoP bytes preserved through EQ/gain guards with preset untouched; failed output config preserves old path; disabled/rescue/kernel/ABI gates");
 return 0;
}
