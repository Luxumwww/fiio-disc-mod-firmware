/* Exercise the production flush with fixed-address vendor memory and mocked
 * ioctl/native callbacks. No device access. */
static unsigned long test_fn(unsigned address);
#define FN(a,t) ((t)test_fn(a))
#define ioctl test_ioctl
#include "provider.c"
#undef ioctl
extern int puts(const char *),printf(const char *,...);
static int last_chunk,ready_calls,native_calls,pans,paints,test_failed;
static unsigned *base;
#define CHECK(x) do {if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);test_failed=1;return 1;}}while(0)
static int is_last(void *d){(void)d;return last_chunk;}
static void ready(void *d){(void)d;ready_calls++;}
static void native(void *d,int *r,void *p){(void)d;(void)r;(void)p;native_calls++;}
static unsigned long test_fn(unsigned a){if(a==0x51ab20)return (unsigned long)is_last;if(a==0x51ab0c)return (unsigned long)ready;return 0;}
int test_ioctl(int fd,unsigned command,...){
 (void)fd;__builtin_va_list ap;__builtin_va_start(ap,command);void *arg=__builtin_va_arg(ap,void *);__builtin_va_end(ap);
 if(command==0x4606){pans++;if(DU(0x8e0980)!=DU(0x8e0914)*360)test_failed=1;}
 if(command==0xc0044702u)memcpy(arg,dpu_trial,sizeof dpu_trial);
 return 0;
}
static void paint(unsigned *p,unsigned f,void *ctx){(void)ctx;paints++;p[0]=0xAA000000u|f;}
static unsigned source[64800];
int main(void){
 if(mmap((void *)0x830000,0xc0000,3,0x32,-1,0)!=(void *)0x830000)return 2;
 if(mmap((void *)0x10000000,3*518400,3,0x32,-1,0)!=(void *)0x10000000)return 2;
 if(mmap((void *)0x11000000,4096,3,0x32,-1,0)!=(void *)0x11000000)return 2;
 if(mmap((void *)0x12000000,3*518400,3,0x32,-1,0)!=(void *)0x12000000)return 2;
 void *driver=(void *)0x11000000;base=(void *)0x10000000;dpu_pixels=(void *)0x12000000;
 DU(0x8e0910)=518400;DU(0x8e0918)=1;DU(0x8e0954)=1440;DU(0x8e096c)=DU(0x8e0970)=360;DU(0x8e0984)=32;DU(0x8e0924)=0x10000000;DU(0x83abd0)=1;
 dpu_native=native;owner_paint=paint;dpu_on=1;
 for(unsigned rotation=1;rotation<=3;rotation+=2){
  DU(0x1100001c)=0x410c|(rotation<<4);DU(0x8e0914)=0;DU(0x83abd8)=1;DU(0x83abd4)=1;dpu_nprevious=-1;dpu_npending=0;
  for(unsigned i=0;i<129600;i++){base[i]=7;base[129600+i]=8;}
  int upper[]={0,0,359,179},lower[]={0,180,359,359};
  for(unsigned i=0;i<64800;i++)source[i]=11;
  last_chunk=0;int old_pans=pans,old_paints=paints;dpu_flush(driver,upper,source);
  CHECK(pans==old_pans&&paints==old_paints&&DU(0x8e0914)==0);
  for(unsigned i=0;i<129600;i++)CHECK(base[i]==7);
  for(unsigned i=0;i<64800;i++)source[i]=22;
  last_chunk=1;dpu_flush(driver,lower,source);
  CHECK(pans==old_pans+1&&paints==old_paints+1&&DU(0x8e0914)==1&&DU(0x83abd8)==0&&native_calls==0);
  for(unsigned i=0;i<129600;i++){CHECK(base[i]==7);CHECK(base[129600+i]==(i<64800?11u:22u));}
  int partial[]={100,110,102,113};for(unsigned i=0;i<12;i++)source[i]=33;dpu_flush(driver,partial,source);
  CHECK(DU(0x8e0914)==0&&base[110*360+100]==33&&base[110*360+99]==11&&base[200*360]==22);
  CHECK(dpu_nprevious==1&&DU(0x83abd4)==1);
 }
 CHECK(ready_calls==6&&!test_failed);
 /* Clipping preserves source stride and both guard pixels. */
 unsigned clipped[129602];for(unsigned i=0;i<129602;i++)clipped[i]=99;int r[]={-1,-1,1,1};unsigned small[]={1,2,3,4,5,6,7,8,9};quarter_copy(clipped+1,r,small);
 CHECK(clipped[0]==99&&clipped[129601]==99&&clipped[1]==5&&clipped[2]==6&&clipped[361]==8&&clipped[362]==9);
 DU(0x1100001c)=0x412c;dpu_flush(driver,r,small);CHECK(native_calls==1);
 DU(0x8e0984)=16;DU(0x1100001c)=0x411c;dpu_flush(driver,r,small);CHECK(native_calls==2);
 puts("PASS: production 90/270 flush never writes scanned front; all chunks precede one pan; next partial frame preserves damage; ready callback/rotation fallback/clipping guards valid");return 0;
}
