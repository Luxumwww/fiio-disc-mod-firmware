/* CS43131 DS1155F2 4.1.9,4.9.11,5.6,5.7.2. Written independently.
 * Board analogue/load/clock configuration is preserved, never reset/copied
 * from a stock DSD function. Both chips are preflighted before any write. */
#ifndef DISC_BACKEND_DAC_H
#define DISC_BACKEND_DAC_H
typedef struct {
 void *ctx;int (*read)(void *,unsigned,unsigned,unsigned char *);
 int (*write)(void *,unsigned,unsigned,unsigned char);void (*wait)(unsigned);
 unsigned char saved[2][37];unsigned saved_ok,configured,up,error,clock_changed;
 unsigned phase,failure_kind,error_chip,pending,polls;
 unsigned char last_irq[2],before_power[2],after_power[2],after_mask[2];
} DsbDac;
static const unsigned dsb_regs[37]={0x20000,0xf0010,0x10006,0x1000b,0x1000c,0x40014,0x40015,0x40016,0x40017,0x40018,0x40019,0x50000,0x50001,0x5000a,0x5000b,0x70000,0x70001,0x70002,0x70003,0x70004,0x70005,0x70006,0x10010,0x80032,0x90004,0x30001,0x40002,0x30002,0x30003,0x30004,0x30005,0x30008,0x3001b,0x3000a,0x80046,0x90003,0xb0000};
static int dsb_get(DsbDac *d,unsigned c,unsigned r,unsigned char *v){if(d->read(d->ctx,0x30+2*c,r,v))return 1;d->error=r;d->failure_kind=1;d->error_chip=c;return 0;}
static int dsb_set(DsbDac *d,unsigned c,unsigned r,unsigned char v){
 unsigned char check=0;if(!d->write(d->ctx,0x30+2*c,r,v)){d->error=r;d->failure_kind=2;d->error_chip=c;return 0;}
 if(!dsb_get(d,c,r,&check))return 0;
 if(check!=v){d->error=r;d->failure_kind=3;d->error_chip=c;return 0;}return 1;
}
static int dsb_down(DsbDac *d){
 unsigned char p[2],m[2],v,hidden[2],pump[2],mute[2];unsigned pending=0,external=0,mute_reg[2]={0,0};d->phase=10;d->pending=d->polls=0;
 for(unsigned c=0;c<2;c++)if(!dsb_get(d,c,0x20000,&p[c])||!dsb_get(d,c,0xf0010,&m[c]))return 0;
 /* The actual board has CLASS_H=0x0b: EXT_VCPFILT is enabled. Section5.8,
  * not the ordinary5.6 shutdown, requires hidden pop-free settings and
  * a completed150ms mute ramp before the HP power-down request. */
 for(unsigned c=0;c<2;c++)if(!(p[c]&0x10)){
  unsigned char classh,dsd;
  if(!dsb_get(d,c,0xb0000,&classh))return 0;
  if(classh&1){
   d->phase=15;external|=1u<<c;
   if(!dsb_get(d,c,0x10010,&hidden[c])||!dsb_get(d,c,0x80046,&pump[c])||!dsb_get(d,c,0x70004,&dsd))return 0;
   mute_reg[c]=(dsd&0x10)?0x70002:0x90003;
   if(!dsb_get(d,c,mute_reg[c],&mute[c])||!dsb_set(d,c,0x10010,0x99)||!dsb_set(d,c,0x80046,0x21)||!dsb_set(d,c,mute_reg[c],mute[c]|3))return 0;
  }
 }
 if(external){d->phase=16;d->wait(150000);}
 for(unsigned c=0;c<2;c++){
  d->before_power[c]=p[c];d->phase=11;
  if(!dsb_set(d,c,0xf0010,m[c]&0xfe))return 0;
  d->after_mask[c]=m[c]&0xfe;
  if(!(p[c]&0x10)){
   d->phase=12;
   if(!dsb_get(d,c,0xf0000,&v)||!dsb_set(d,c,0x20000,p[c]|0x10))return 0;
   d->after_power[c]=p[c]|0x10;
   pending|=1u<<c;
  }
 }
 /* PDN_DONE is required before touching the serial interface. */
 d->phase=13;
 for(unsigned count=0;pending&&count<150;count++){
  d->polls=count+1;d->pending=pending;
  d->wait(1000);for(unsigned c=0;c<2;c++)if(pending&(1u<<c)){
   if(!dsb_get(d,c,0xf0000,&v))return 0;d->last_irq[c]=v;if(v&1)pending&=~(1u<<c);
  }
 }
 d->pending=pending;
 if(pending){d->error=0xf0000;d->failure_kind=4;return 0;}
 d->phase=14;
 for(unsigned c=0;c<2;c++)if(!dsb_get(d,c,0x20000,&v)||!dsb_set(d,c,0x20000,v|0xe0))return 0;
 for(unsigned c=0;c<2;c++)if(external&(1u<<c)){
  if(!dsb_set(d,c,mute_reg[c],mute[c])||!dsb_set(d,c,0x80046,pump[c])||!dsb_set(d,c,0x10010,hidden[c]))return 0;
 }
 d->up=0;return 1;
}
static int dsb_snapshot(DsbDac *d){
 d->saved_ok=d->configured=d->up=d->error=d->clock_changed=d->failure_kind=0;d->phase=1;
 for(unsigned c=0;c<2;c++){
  unsigned char id[3];for(unsigned i=0;i<3;i++)if(!dsb_get(d,c,0x10000+i,&id[i]))return 0;
  if(id[0]!=0x43||id[1]!=0x13||id[2]!=0x10){d->error=0x10000;return 0;}
  for(unsigned i=0;i<37;i++)if(!dsb_get(d,c,dsb_regs[i],&d->saved[c][i]))return 0;
  unsigned clock=d->saved[c][2]&7;
  /* Current board PCM uses external22.5792MHz (source4). Independently
   * convert that known input to async24.576MHz using Table4-4. */
  if(clock!=0&&clock!=1&&clock!=4){d->error=0x10006;return 0;}
  if(d->saved[c][9]!=0x0c||d->saved[c][10]!=0x0a){d->error=0x40018;return 0;}
 }
 d->saved_ok=1;return 1;
}
static int dsb_clock(DsbDac *d){
 unsigned pending=0;unsigned char p,m,v;d->phase=20;
 for(unsigned c=0;c<2;c++)if((d->saved[c][2]&7)==4){
  d->clock_changed|=1u<<c;
  /* HP/ports are already off. Move MCLK to RCO before stopping/reprogramming
   * PLL, then wait for both READY events before selecting either output. */
  if(!dsb_set(d,c,0x10006,(d->saved[c][2]&~3)|2))return 0;d->wait(150);
  if(!dsb_set(d,c,0x30001,0)||!dsb_get(d,c,0x20000,&p)||!dsb_set(d,c,0x20000,p&~4))return 0;
  const unsigned regs[]={0x40002,0x30008,0x30002,0x30003,0x30004,0x30005,0x3001b,0x3000a};
  const unsigned char vals[]={3,8,0,0xf7,6,0x44,(unsigned char)(d->saved[c][32]&~2),139};
  for(unsigned i=0;i<8;i++)if(!dsb_set(d,c,regs[i],vals[i]))return 0;
  if(!dsb_get(d,c,0xf0010,&m)||!dsb_set(d,c,0xf0010,m&~6)||!dsb_get(d,c,0xf0000,&v)||!dsb_set(d,c,0x30001,1))return 0;
  pending|=1u<<c;
 }
 for(unsigned count=0;pending&&count<100;count++){
  d->wait(1000);for(unsigned c=0;c<2;c++)if(pending&(1u<<c)){
   if(!dsb_get(d,c,0xf0000,&v))return 0;
   if(v&2){d->error=0x30001;return 0;}if(v&4)pending&=~(1u<<c);
  }
 }
 if(pending){d->error=0x30001;return 0;}
 for(unsigned c=0;c<2;c++)if(d->clock_changed&(1u<<c)){
  if(!dsb_set(d,c,0x10006,(d->saved[c][2]&~7)|1))return 0;d->wait(150);
 }
 return 1;
}
static int dsb_setup(DsbDac *d,unsigned rate){
 if(rate!=176400&&rate!=352800)return 0;
 if(!dsb_snapshot(d)||!dsb_down(d)||!dsb_clock(d))return 0;
 for(unsigned c=0;c<2;c++){
  /* Map PCM channel swap/copy/inversion to DSD, retaining board wiring.
   * SIGCTL_DSDEQPCM stays off: DSD has an independent mute and volume. */
  unsigned char map=d->saved[c][24]&15;
  const unsigned regs[]={0x1000b,0x1000c,0x40014,0x40015,0x40016,0x40017,0x50000,0x50001,0x5000a,0x5000b,0x70002,0x70003,0x70004,0x70005,0x70006};
  const unsigned char vals[]={
   (d->saved[c][3]&0xf0)|(rate==176400?5:7),
   (d->saved[c][4]&0xfc)|1,31,0,63,0,d->saved[c][11],d->saved[c][12],
   (d->saved[c][13]&8)|6,(d->saved[c][14]&8)|6,
   0xeb,0,(rate==176400?0x52:0x56),0,(unsigned char)(0xc0|map)};
  for(unsigned i=0;i<sizeof(regs)/sizeof(regs[0]);i++)if(!dsb_set(d,c,regs[i],vals[i]))return 0;
 }
 d->configured=1;return 1;
}
static int dsb_serial_up(DsbDac *d){
 if(!d->configured)return 0;
 for(unsigned c=0;c<2;c++){
  unsigned char p;if(!dsb_get(d,c,0x20000,&p)||!dsb_set(d,c,0x10010,0x99)||!dsb_set(d,c,0x80032,0x20)||!dsb_set(d,c,0x20000,(p|0x10)&0xbf))return 0;
 }return 1;
}
static int dsb_hp_up(DsbDac *d){
 for(unsigned c=0;c<2;c++){unsigned char p;if(!dsb_get(d,c,0x20000,&p)||!dsb_set(d,c,0x20000,p&0xef))return 0;}
 d->wait(12000);
 for(unsigned c=0;c<2;c++)if(!dsb_set(d,c,0x80032,0)||!dsb_set(d,c,0x10010,0)||!dsb_set(d,c,0x70002,0xe8))return 0;
 d->up=1;return 1;
}
static int dsb_volume(DsbDac *d,unsigned c,unsigned char v){return dsb_set(d,c,0x70000,v)&&dsb_set(d,c,0x70001,v);}
static int dsb_restore(DsbDac *d){
 if(!d->saved_ok)return 1;if(!dsb_down(d))return 0;
 for(unsigned c=0;c<2;c++){
  if(d->clock_changed&(1u<<c)){
   if(!dsb_set(d,c,0x10006,(d->saved[c][2]&~3)|2))return 0;d->wait(150);
   if(!dsb_set(d,c,0x30001,0))return 0;
   for(unsigned i=26;i<34;i++)if(!dsb_set(d,c,dsb_regs[i],d->saved[c][i]))return 0;
   unsigned char p;if(!dsb_get(d,c,0x20000,&p)||!dsb_set(d,c,0x20000,(p&~4)|(d->saved[c][0]&4)))return 0;
   if(!dsb_set(d,c,0x30001,d->saved[c][25])||!dsb_set(d,c,0x10006,d->saved[c][2]))return 0;d->wait(150);
  }
  for(unsigned i=3;i<25;i++)if(!dsb_set(d,c,dsb_regs[i],d->saved[c][i]))return 0;
  for(unsigned i=34;i<36;i++)if(!dsb_set(d,c,dsb_regs[i],d->saved[c][i]))return 0;
  if(!dsb_set(d,c,0xf0010,d->saved[c][1]))return 0;
  /* Leave HP safely off. Native PCM startup may then perform its usual
   * power-up; no DSD/PCM content is played during restoration. */
  if(!dsb_set(d,c,0x20000,d->saved[c][0]|0x10))return 0;
 }
 d->saved_ok=d->configured=d->up=d->clock_changed=0;return 1;
}
#endif
