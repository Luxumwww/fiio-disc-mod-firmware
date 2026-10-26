/* Test-only preload for QEMU: calls pinned stock PCM gain, then exits before
 * mq_player main. No ALSA device, control file, UI or playback is opened. */
extern int printf(const char *,...);
extern void _exit(int);
extern int fflush(void *);
#define W(a) (*(volatile unsigned *)(a))
#define B(a) (*(volatile unsigned char *)(a))
#define F(a) (*(volatile float *)(a))
static unsigned route[256],player[64];
__attribute__((constructor)) static void test(void){
 W(0x83dfe4)=(unsigned)route;route[0x58/4]=3;
 W(0x898a6c)=0;W(0x83a704)=0;W(0x839618)=0;W(0x83961c)=0;
 void (*volume)(unsigned)=(void *)0x4ea5d4;
 void (*process)(void *,int,int,void *)=(void *)0x44dca4;
 int fail=0;
 B(0x83a78a)=B(0x83a78b)=B(0x83a799)=0;B(0x83a75a)=3;
 B(0x83a74c)=9;B(0x83a7a0)=90;
 unsigned (*get)(void)=(void *)0x4ea718;void (*down)(void)=(void *)0x4ec02c;
 if(get()!=9)fail=1;down();if(get()!=8||B(0x83a7a0)!=90)fail=1;
 printf("stock pure volume key: local=%u BT=%u\n",get(),B(0x83a7a0));
 B(0x83a799)=3;if(get()!=90)fail=1;down();if(get()!=89||B(0x83a74c)!=8)fail=1;
 printf("stock Bluetooth volume key: BT=%u local=%u\n",get(),B(0x83a74c));
 for(int bits=16;bits<=32;bits+=16){
  for(int percent=0;percent<=100;percent+=25){
   volume(percent);float db=F(0x839620);
   int buf32[4]={1000000000,-1000000000,500000000,-500000000};
   short buf16[4]={30000,-30000,15000,-15000};
   process(bits==16?(void *)buf16:(void *)buf32,bits==16?8:16,bits,player);
   int sample=bits==16?buf16[0]:buf32[0];
   /* Stock0 is -116.5dB, rather than exact mathematical silence in32bit. */
   if(!percent&&(sample>(bits==16?1:2000)||sample<-(bits==16?1:2000)||db>-110))fail=1;
   if(percent==100&&sample!=(bits==16?30000:1000000000))fail=1;
   if(percent>0&&percent<100&&(sample<=0||sample>=(bits==16?30000:1000000000)))fail=1;
   printf("stock PCM bits=%d percent=%d db=%.1f sample=%d\n",bits,percent,(double)db,sample);
  }
 }
 printf("%s: real stock gain, no audio output\n",fail?"FAIL":"PASS");fflush(0);_exit(fail);
}
