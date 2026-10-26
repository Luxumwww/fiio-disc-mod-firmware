/* Reuse exact native mocks; test the new profile separately from legacy tests. */
#define main legacy_test_main
#include "test_dop_compat.c"
#undef main
static unsigned stock_calls;
static void stock_native(unsigned rate,unsigned bits,unsigned dsd){
 assert((rate==176400||rate==352800)&&bits==32&&dsd==1);
 stock_calls++;PC_BYTE(0x899060)=1;PC_WORD(0x899084)=rate;
}
/* Native mock address resolves to repaired() in the shared test. It uses
 * simulated ioctl, so supply a direct call wrapper for this profile only. */
int main(void){
 unsigned p=0x810000;stock_feature=1;disabled=1;mode=1;
 PC_WORD(0x83a6bc)=0x4e9534;PC_WORD(0x83a704)=0x4e94f4;
 dop_install();assert(!dop_original_configure&&!PC_WORD(PLAYER_IOCTL_GOT));
 disabled=0;mode=4;clock_sec++;PC_WORD(0x83dfc4)=p;
 PC_BYTE(0x83a75a)=6;PC_WORD(0x839a74)=1;
 PC_WORD(p+0xc)=PC_WORD(p+0x1c)=352800;PC_WORD(p+0x10)=PC_WORD(p+0x20)=2;
 PC_WORD(p+0x14)=PC_WORD(p+0x24)=32;PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;PC_WORD(p+0x98)=2;
 PC_WORD(0x83a7d0)=8;PC_WORD(0x83a7cc)=9;
 dop_install();assert(dop_stock_profile&&dop_enabled&&dop_eq_ready);
 assert(!dop_real_ioctl&&!PC_WORD(PLAYER_IOCTL_GOT));
 /* This mock skips same-format bus activity to isolate module transactions.
  * The real native function may still write volume; this is not HW proof. */
 native_noop=1;PC_BYTE(0x899060)=1;PC_WORD(0x899084)=352800;
 assert(dop_configure(352800,32,1)==0&&fix_calls==1&&!dop_bus_writes&&!rate_closes);
 unsigned packet[4]={0x05aabbcc,0x05ddeeff,0xfa123456,0xfa987654},copy[4];memcpy(copy,packet,sizeof copy);
 assert(!dop_eq_filter(packet,sizeof packet,352800)&&!memcmp(packet,copy,sizeof copy));
 assert(!dop_gain_filter()&&!native_gain_calls);
 assert(dop_configure(96000,32,1)==17&&!rate_closes&&!dop_rate_repairs);
 mode=1;assert(dop_configure(352800,32,1)==17);
 mode=2;assert(dop_configure(352800,32,1)==17);
 mode=4;route=3;assert(dop_configure(352800,32,1)==17);route=6;
 PC_WORD(p+0x2c)=1;assert(dop_eq_filter(packet,sizeof packet,352800)==sizeof packet);
 assert(dop_configure(352800,32,0)==17);
 disabled=1;clock_sec++;dop_gate_refresh();PC_WORD(p+0x2c)=2;
 assert(!dop_enabled&&dop_configure(352800,32,1)==17);
 (void)stock_native;
 puts("PASS: working-local configuration reused; disabled startup/local/BT/PCM/USB-output gates; EQ/gain bypass preserves DoP; no I2C override/PCM reopen/import hook/recovery");
}
