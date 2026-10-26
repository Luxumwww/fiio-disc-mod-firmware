#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stream.h"
#include "dac.h"
static unsigned char regs[2][0x100000];
static unsigned hp_done[2],pll_done[2],muted_at[2],elapsed,writes,reject_at,stall,pll_stall,serial_early;
static int rd(void *ctx,unsigned a,unsigned r,unsigned char *v){
 (void)ctx;unsigned c=(a-0x30)/2;if(r==0xf0000){*v=0;if(hp_done[c]&&elapsed>=hp_done[c]&&!stall){*v|=1;hp_done[c]=0;}if(pll_done[c]&&elapsed>=pll_done[c]&&!pll_stall){*v|=4;pll_done[c]=0;}return 1;}
 *v=regs[c][r];return 1;
}
static int wr(void *ctx,unsigned a,unsigned r,unsigned char v){
 (void)ctx;unsigned c=(a-0x30)/2;writes++;if(reject_at&&writes==reject_at)return 0;
 if(r==0x20000){if((v&0x40)&&!(regs[c][r]&0x40)&&hp_done[c])serial_early++;
  if((v&0x10)&&!(regs[c][r]&0x10)){
   if(regs[c][0xb0000]&1){assert(regs[c][0x10010]==0x99&&regs[c][0x80046]==0x21);assert(elapsed>=muted_at[c]+150000);}
   hp_done[c]=elapsed+4000;
  }
 }
 if(r==0x30001&&v==1)pll_done[c]=elapsed+3000;
 if((r==0x90003||r==0x70002)&&(v&3)==3)muted_at[c]=elapsed;
 regs[c][r]=v;return 1;
}
static void delay(unsigned n){elapsed+=n;}
static DsbDac fresh(void){
 memset(regs,0,sizeof(regs));hp_done[0]=hp_done[1]=pll_done[0]=pll_done[1]=elapsed=writes=reject_at=stall=pll_stall=serial_early=0;
 for(unsigned c=0;c<2;c++){
  regs[c][0x10000]=0x43;regs[c][0x10001]=0x13;regs[c][0x10002]=0x10;
  regs[c][0x10006]=1;regs[c][0x40018]=12;regs[c][0x40019]=10;
  regs[c][0x20000]=0xa0;regs[c][0xf0010]=255;regs[c][0x90004]=c?3:1;
  regs[c][0x5000a]=regs[c][0x5000b]=c?15:7;
 }
 DsbDac d={0};d.read=rd;d.write=wr;d.wait=delay;return d;
}
int main(void){
 unsigned char in[4096],out[4096],again[4096];DsbStream s;dsb_reset(&s);
 for(unsigned i=0;i<sizeof(in);i+=8){in[i]=in[i+4]=0;in[i+3]=in[i+7]=(i/8&1)?250:5;
  in[i+1]=(unsigned char)i;in[i+2]=(unsigned char)(i/3);in[i+5]=(unsigned char)(255-i);in[i+6]=(unsigned char)(i/7);}
 assert(dsb_scan(&s,in,15*8)==1);assert(dsb_scan(&s,in+15*8,17*8)==2);
 assert(s.seams==0);dsb_frame(&s,in,31*8,out);dsb_frame(&s,in+31*8,8,again);
 assert(out[30*8+3]==5&&again[3]==250);
 for(unsigned i=0;i<31*8;i+=8)assert(!memcmp(out+i+1,in+i+1,2)&&!memcmp(out+i+5,in+i+5,2));
 assert(dsb_scan(&s,in,33*8)==2);assert(dsb_scan(&s,in,sizeof(in))==2&&s.seams==1);
 in[7]=17;assert(dsb_scan(&s,in,sizeof(in))==0);assert(dsb_marked(in,sizeof(in)));
 dsb_frame(&s,0,sizeof(out),out);assert(out[1]==0x69&&out[2]==0x69);assert(!dsb_zero(out,sizeof(out)));
 memset(out,0,sizeof(out));assert(dsb_zero(out,sizeof(out)));assert(dsb_scan(&s,in,3)==-1);
 DsbDac d=fresh();assert(dsb_setup(&d,352800));assert(!serial_early&&elapsed>=4000);
 assert(regs[0][0x1000c]==1&&regs[0][0x5000a]==6&&regs[1][0x5000b]==14);
 assert(regs[0][0x70004]==0x56&&regs[1][0x70006]==0xc3);
 assert(dsb_volume(&d,0,40));assert(dsb_serial_up(&d)&&!serial_early);assert(regs[0][0x20000]&0x10);
 assert(dsb_hp_up(&d));assert(!(regs[0][0x20000]&0x10));assert(elapsed>=16000);
 assert(dsb_restore(&d));assert(regs[0][0x20000]==0xb0&&regs[0][0x70004]==0);
 d=fresh();assert(dsb_setup(&d,176400)&&regs[1][0x70004]==0x52);assert(dsb_restore(&d));
 d=fresh();stall=1;assert(!dsb_setup(&d,352800));assert(d.error==0xf0000&&!serial_early);
 assert((regs[0][0x20000]&0x50)==0x10&&(regs[1][0x20000]&0x50)==0x10);
 d=fresh();regs[1][0x10001]=0;assert(!dsb_setup(&d,352800)&&writes==0);
 d=fresh();regs[1][0x10006]=5;assert(!dsb_setup(&d,352800)&&writes==0);
 d=fresh();reject_at=8;assert(!dsb_setup(&d,352800));assert(regs[0][0x20000]&0x10);assert(regs[1][0x20000]&0x10);
 assert(dsb_restore(&d));assert(regs[0][0x20000]&0x10);
 d=fresh();regs[0][0x10006]=regs[1][0x10006]=4;
 assert(dsb_setup(&d,352800));assert(regs[0][0x10006]==1&&regs[1][0x10006]==1);
 assert(regs[0][0x30005]==0x44&&regs[0][0x30003]==0xf7&&!(regs[0][0x3001b]&2));
 assert(dsb_restore(&d));assert(regs[0][0x10006]==4&&regs[1][0x10006]==4&&regs[0][0x30005]==0);
 d=fresh();regs[0][0x10006]=regs[1][0x10006]=4;pll_stall=1;
 assert(!dsb_setup(&d,176400)&&d.error==0x30001);assert((regs[0][0x20000]&0x10)&&(regs[1][0x20000]&0x10));
 assert(dsb_restore(&d));assert(regs[0][0x10006]==4&&regs[1][0x10006]==4);
 d=fresh();for(unsigned c=0;c<2;c++){regs[c][0xb0000]=0x0b;regs[c][0x80046]=0x20;regs[c][0x90003]=0xac;}
 assert(dsb_setup(&d,352800));assert(elapsed>=154000&&regs[0][0x90003]==0xac&&regs[0][0x80046]==0x20&&regs[0][0x10010]==0);
 assert(dsb_serial_up(&d)&&dsb_hp_up(&d));assert(dsb_restore(&d));
 assert(regs[1][0x90003]==0xac&&regs[1][0x80046]==0x20&&regs[1][0x10010]==0);
 puts("PASS: split DoP detection, stereo byte preservation, continuous markers, corrupt-data guard, DSD64/128, dual-DAC HP sequencing, timeout and partial-write restoration.");return 0;
}
