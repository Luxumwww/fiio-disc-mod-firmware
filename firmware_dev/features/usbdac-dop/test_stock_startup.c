/* Startup readiness migration, using the real stock-profile tick/gates. */
#define main legacy_test_main
#include "test_dop_compat.c"
#undef main
static unsigned configured;
static void stock_config(unsigned rate,unsigned bits,unsigned dsd){
 assert((rate==176400||rate==352800)&&bits==32&&dsd==1);
 configured++;PC_BYTE(0x899060)=1;PC_WORD(0x899084)=rate;
}
int main(void){
 unsigned p=0x810000;stock_feature=1;native_config_override=stock_config;
 PC_WORD(0x83dfc4)=p;PC_WORD(0x83a6bc)=0x4e9534;PC_WORD(0x83a704)=0x4e94f4;
 PC_BYTE(0x83a75a)=6;PC_WORD(0x839a74)=1;
 PC_WORD(p+0x10)=PC_WORD(p+0x20)=2;PC_WORD(p+0x14)=PC_WORD(p+0x24)=32;
 PC_WORD(p+0x18)=0;PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;PC_WORD(p+0x98)=2;
 PC_WORD(0x83a7d0)=8;PC_WORD(0x83a7cc)=9;
 PC_WORD(p+0xc)=PC_WORD(p+0x1c)=352800;dop_install();
 assert(dop_enabled&&dop_stock_profile&&dop_eq_ready);
 for(unsigned rate=176400;rate<=352800;rate*=2){
  PC_WORD(p+0x2c)=1;PC_WORD(p+0x58)=0;dop_tick();
  PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;
  PC_WORD(p+0xc)=PC_WORD(p+0x1c)=rate;PC_BYTE(0x899060)=0;
  unsigned before=configured,native_before=native_calls;
  open_pcm=0;for(unsigned i=0;i<20;i++)dop_tick();
  assert(dop_startup_pending&&dop_startup_wait_reason==2&&!dop_repair_attempted);
  assert(configured==before&&native_calls==native_before);
  open_pcm=1;PC_WORD(p+0x1c)=96000;dop_tick();
  assert(dop_startup_wait_reason==4&&!dop_repair_attempted&&configured==before);
  PC_WORD(p+0x1c)=rate;PC_WORD(0x83a7cc)=(unsigned)-1;dop_tick();
  assert(dop_startup_wait_reason==8&&!dop_repair_attempted&&configured==before);
  PC_WORD(0x83a7cc)=9;dop_eq_ready=0;dop_eq_attempted=1;dop_tick();
  assert(dop_startup_wait_reason==1&&!dop_repair_attempted&&configured==before);
  dop_eq_ready=1;dop_original_gain=0;dop_tick();
  assert(dop_startup_wait_reason==1&&!dop_repair_attempted&&configured==before);
  dop_original_gain=native_gain;dop_tick();
  assert(configured==before+1&&dop_repair_attempted&&!dop_startup_pending&&!dop_startup_wait_reason);
  for(unsigned i=0;i<100;i++)dop_tick();assert(configured==before+1);
 }
 assert(dop_startup_applied==2&&!dop_bus_writes&&!rate_closes&&!rate_opens&&!native_health);
 /* Same-rate genuine PCM must never consume startup eligibility or force DSD. */
 PC_WORD(p+0x2c)=1;PC_WORD(p+0x58)=0;PC_BYTE(0x899060)=0;
 unsigned before=configured;for(unsigned i=0;i<100;i++)dop_tick();
 assert(configured==before&&!dop_startup_pending&&!dop_repair_attempted);
 PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;open_pcm=0;dop_tick();assert(dop_startup_pending);
 disabled=1;clock_sec++;dop_tick();assert(!dop_enabled&&!dop_startup_pending&&!dop_repair_attempted);
 disabled=0;clock_sec++;mode=1;dop_tick();assert(!dop_startup_pending&&configured==before);
 mode=4;route=3;dop_tick();assert(!dop_startup_pending&&configured==before);
 route=6;PC_WORD(p+0x98)=3;dop_tick();assert(!dop_startup_pending&&configured==before);
 PC_WORD(p+0x98)=2;open_pcm=1;dop_tick();assert(configured==before+1);
 assert(!PC_WORD(PLAYER_IOCTL_GOT)&&!dop_real_ioctl&&!dop_bus_writes&&!rate_closes);
 puts("PASS: DoP64/128 waits for output/rate/handles/EQ/gain; pending survives readiness; config exactly once; PCM/local/BT/USB/disabled/strategy gates; no hardware poll/reopen");
}
