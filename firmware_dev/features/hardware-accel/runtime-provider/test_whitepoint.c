/* Production provider + mocked kernel. No device or audio access. */
static unsigned long test_fn(unsigned);
#define FN(a,t) ((t)test_fn(a))
#define ioctl test_ioctl
#define open test_open
#define close test_close
#define mmap test_mmap
#include "provider.c"
#undef ioctl
#undef open
#undef close
#undef mmap
extern void *mmap(void *,unsigned,int,int,int,long);
extern int printf(const char *,...),puts(const char *);
static unsigned actual[72],stock[72];
static int gets,puts_cfg,pans,clears,fail_put,fail_get,fail_pan,fail_get_after_put,corrupt_put,inject_external;
static unsigned char *black,*overlay;
static void *display_object,*driver;
static int native_calls,paint_calls;
#define CHECK(x) do{if(!(x)){printf("FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
static void native(void *a,int *b,void *c){(void)a;(void)b;(void)c;native_calls++;}
static void paint(unsigned *p,unsigned f,void *c){(void)c;paint_calls++;p[0]=0xff000000u|f;}
static void *get_display(void){return display_object;}
static int last(void *d){(void)d;return 1;}
static void ready(void *d){(void)d;}
static unsigned long test_fn(unsigned a){if(a==0x51a7e0)return (unsigned long)get_display;if(a==0x565dcc)return (unsigned long)native;if(a==0x51ab20)return (unsigned long)last;if(a==0x51ab0c)return (unsigned long)ready;return 0;}
int test_open(const char *path,int mode,...){(void)mode;return path[7]=='2'?12:path[7]=='1'?11:-1;}
int test_close(int fd){(void)fd;return 0;}
void *test_mmap(void *a,unsigned bytes,int b,int c,int fd,long off){(void)a;(void)bytes;(void)b;(void)c;(void)off;if(fd==12){clears++;return black;}if(fd==11)return overlay;return (void *)-1;}
int test_ioctl(int fd,unsigned cmd,...){
 __builtin_va_list ap;__builtin_va_start(ap,cmd);void *arg=__builtin_va_arg(ap,void *);__builtin_va_end(ap);
 if(cmd==0xc0044702u){gets++;if(fail_get){fail_get--;return -1;}memcpy(arg,actual,sizeof actual);return 0;}
 if(cmd==0xc0044701u){puts_cfg++;if(fail_put){fail_put--;return -1;}memcpy(actual,arg,sizeof actual);if(corrupt_put){corrupt_put--;actual[37]=359;}if(inject_external){inject_external--;actual[54]^=2;}if(fail_get_after_put){fail_get_after_put--;fail_get=1;}return 0;}
 if(cmd==0x4606){pans++;if(fail_pan){fail_pan--;return -1;}return 0;}
 if(cmd==0x80044620u)return 0;
 if(cmd==0x4602){unsigned *p=arg;memset(p,0,128);p[4]=0x07800000u+(fd==12?4147200u:fd==11?2073600u:0u);p[5]=1555200;p[11]=1440;return 0;}
 return -1;
}
static void reset(void){
 memcpy(actual,stock,sizeof actual);wp_value=wp_armed=wp_fault=wp_recovered_visual=0;wp_pixels=0;wp_fd=-1;dpu_on=dpu_failed=0;dpu_fd=-1;dpu_pixels=0;dpu_driver=0;dpu_nprevious=-1;dpu_npending=0;ownership.token=ownership.sequence=0;owner_paint=0;owner_context=0;provider_enabled=provider_guarded=1;
 fail_put=fail_get=fail_pan=fail_get_after_put=corrupt_put=inject_external=0;DU(0x83abd0)=1;DU(0x8e0984)=32;
 if(driver)*(void **)((char *)driver+0x20)=(void *)0x565dcc;
}
int main(void){
 if(mmap((void *)0x830000,0xc0000,3,0x32,-1,0)!=(void *)0x830000)return 2;
 if(mmap((void *)0x10000000,3*518400,3,0x32,-1,0)!=(void *)0x10000000)return 2;
 if(mmap((void *)0x11000000,4096,3,0x32,-1,0)!=(void *)0x11000000)return 2;
 if(mmap((void *)0x12000000,3*518400+4096,3,0x32,-1,0)!=(void *)0x12000000)return 2;
 if(mmap((void *)0x13000000,3*518400+4096,3,0x32,-1,0)!=(void *)0x13000000)return 2;
 overlay=(void *)0x12000000;black=(void *)0x13000000;display_object=(void *)0x11000100;driver=(void *)0x11000200;*(void **)display_object=driver;
 DU(0x8e0910)=518400;DU(0x8e0918)=1;DU(0x8e0954)=1440;DU(0x8e096c)=DU(0x8e0970)=360;DU(0x8e0984)=32;DU(0x8e0924)=0x10000000;DU(0x83abd0)=1;DU(0x8e0914)=0;DU(0x83abd8)=1;DU(0x8e091c)=10;
 unsigned words[]={0x003fc119,0x003fc110,0x003fc108,0x003fc100};for(unsigned l=0;l<4;l++){stock[l*18]=words[l];stock[l*18+1]=stock[l*18+2]=stock[l*18+3]=360;}
 reset();CHECK(disc_accel_whitepoint_get()==0);CHECK(disc_accel_whitepoint_set(0)==0&&puts_cfg==0&&clears==0);
 for(unsigned round=0;round<8;round++)for(unsigned rotation=0;rotation<4;rotation++){
  reset();DU(0x1100021c)=0x410c|(rotation<<4);int old_clears=clears,old_pans=pans;
  CHECK(disc_accel_whitepoint_set(40)==40&&disc_accel_whitepoint_get()==40);CHECK(clears==old_clears+1&&pans==old_pans+1);
  CHECK(((actual[0]>>3)&7)==1&&((actual[18]>>3)&7)==2&&((actual[36]>>3)&7)==3&&((actual[54]>>3)&7)==0);
  CHECK((actual[36]&1)&&((actual[36]>>13)&1)&&((actual[36]>>14)&255)==102&&((actual[36]>>6)&15)==4);
  for(unsigned i=0;i<1555200;i++)CHECK(black[i]==0);
  unsigned token=api_claim("test/page",paint,0);CHECK(token&&dpu_on&&disc_accel_whitepoint_get()==40&&clears==old_clears+1);
  CHECK(((actual[0]>>3)&7)==1&&((actual[18]>>3)&7)==2&&(actual[18]&1));
  old_pans=pans;CHECK(disc_accel_whitepoint_set(80)==80&&pans==old_pans+1&&clears==old_clears+1);
  api_release(token);CHECK(!dpu_on&&disc_accel_whitepoint_get()==80&&!(actual[18]&1));
  CHECK(disc_accel_whitepoint_set(0)==0&&!memcmp(actual,stock,sizeof actual));
 }
 /* Turning the mask off during an active visual restores its exact original
  * configuration and retains its lease/callback. */
 reset();DU(0x1100021c)=0x410c;unsigned token=api_claim("test/page",paint,0);CHECK(token);unsigned visual[72];memcpy(visual,actual,sizeof visual);
 CHECK(disc_accel_whitepoint_set(55)==55&&disc_accel_whitepoint_set(0)==0&&!memcmp(actual,visual,sizeof visual)&&api_valid(token));api_release(token);CHECK(!memcmp(actual,stock,sizeof actual));
 /* Failed GET/PUT, transient readback and pan failure preserve prior setting. */
 reset();fail_get=1;CHECK(disc_accel_whitepoint_set(30)==-1&&!memcmp(actual,stock,sizeof actual));
 fail_put=1;CHECK(disc_accel_whitepoint_set(30)==-1&&!memcmp(actual,stock,sizeof actual)&&disc_accel_whitepoint_get()==0);
 fail_get_after_put=1;CHECK(disc_accel_whitepoint_set(30)==-1&&!memcmp(actual,stock,sizeof actual)&&disc_accel_whitepoint_get()==0);
 fail_pan=1;CHECK(disc_accel_whitepoint_set(30)==-1&&!memcmp(actual,stock,sizeof actual)&&disc_accel_whitepoint_get()==0);
 CHECK(disc_accel_whitepoint_set(25)==25);unsigned prior[72];memcpy(prior,actual,sizeof prior);fail_put=1;CHECK(disc_accel_whitepoint_set(60)==-1&&!memcmp(actual,prior,sizeof prior)&&disc_accel_whitepoint_get()==25);
 /* 200 ms self-check is read-only and foreign configs are never clobbered. */
 int old_puts=puts_cfg;wp_checkat=0;wp_tick();CHECK(puts_cfg==old_puts);actual[54]^=2;CHECK(disc_accel_whitepoint_get()==-1);CHECK(disc_accel_whitepoint_set(0)==-1&&puts_cfg==old_puts);
 reset();inject_external=1;CHECK(disc_accel_whitepoint_set(30)==-1&&wp_fault&&puts_cfg==old_puts+1);
 /* Power-off/shape guards make no writes and leave a valid dim setting. */
 reset();CHECK(disc_accel_whitepoint_set(20)==20);old_puts=puts_cfg;DU(0x83abd0)=0;CHECK(disc_accel_whitepoint_set(30)==-1&&puts_cfg==old_puts&&disc_accel_whitepoint_get()==20);DU(0x83abd0)=1;DU(0x8e0984)=16;CHECK(disc_accel_whitepoint_set(30)==-1&&puts_cfg==old_puts);DU(0x8e0984)=32;CHECK(disc_accel_whitepoint_set(900)==80&&disc_accel_whitepoint_get()==80);
 /* Provider disable/safe mode clears both managed features and rejects a
  * subsequent periodic nonzero write. Zero cleanup also works while asleep. */
 reset();CHECK(disc_accel_whitepoint_set(40)==40);token=api_claim("test/page",paint,0);CHECK(token);api_enabled(0);CHECK(!ownership.token&&!dpu_on&&!memcmp(actual,stock,sizeof actual)&&disc_accel_whitepoint_get()==0);old_puts=puts_cfg;CHECK(disc_accel_whitepoint_set(40)==-1&&puts_cfg==old_puts);api_enabled(1);CHECK(disc_accel_whitepoint_set(40)==40);DU(0x83abd0)=0;api_enabled(0);CHECK(!memcmp(actual,stock,sizeof actual)&&disc_accel_whitepoint_get()==0);DU(0x83abd0)=1;
 /* New mq_ui process: controller config survives, every representable mask
  * setting is taken over without stacking; stale old visual is removed. */
 for(unsigned r=1;r<=80;r++)for(unsigned with_visual=0;with_visual<2;with_visual++){
  reset();memcpy(actual,stock,sizeof actual);if(with_visual)wp_visual(actual,0);wp_mask(actual,r);
  CHECK(disc_accel_whitepoint_get()==(int)r&&wp_armed&&wp_recovered_visual);int old_pans=pans;
  CHECK(disc_accel_whitepoint_set(r)==(int)r&&pans==old_pans+1&&!(actual[18]&1u));
  CHECK(disc_accel_whitepoint_set(0)==0&&!memcmp(actual,stock,sizeof actual));
 }
 reset();wp_mask(actual,40);api_enabled(0);CHECK(!memcmp(actual,stock,sizeof actual)&&disc_accel_whitepoint_get()==0);
 reset();wp_visual(actual,0);wp_mask(actual,40);token=api_claim("first-after-reload",paint,0);CHECK(token&&disc_accel_whitepoint_get()==40);api_release(token);CHECK(!(actual[18]&1u)&&disc_accel_whitepoint_set(0)==0&&!memcmp(actual,stock,sizeof actual));
 for(unsigned old_visual=0;old_visual<2;old_visual++){
  reset();if(old_visual)wp_visual(actual,0);wp_mask(actual,40);fail_get=1;
  CHECK(api_claim("first-get-temporary-failure",paint,0)==0&&!dpu_failed);
  token=api_claim("retry-before-any-whitepoint-get",paint,0);CHECK(token&&disc_accel_whitepoint_get()==40);api_release(token);CHECK(disc_accel_whitepoint_set(0)==0&&!memcmp(actual,stock,sizeof actual));
 }
 puts("PASS: 32 four-rotation mask/visual cycles, 160 controller-surviving restart cases, safe-mode disable, static three-page clear, unchanged-frame pans, active-lease disable, GET/PUT/readback/pan rollback, foreign-config fail-closed, power/shape guards, read-only self-check");return 0;
}
