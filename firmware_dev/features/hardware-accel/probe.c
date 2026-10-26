typedef unsigned size_t;
extern int printf(const char *,...);
extern int memcmp(const void *,const void *,size_t);
extern void *memcpy(void *,const void *,size_t);
extern int clock_gettime(int,void *);
struct ts {long sec,nsec;};
static unsigned now(void){struct ts t;clock_gettime(1,&t);return (unsigned)t.sec*1000000u+(unsigned)t.nsec/1000u;}
static unsigned char a[518400] __attribute__((aligned(16))),b[518400] __attribute__((aligned(16)));
__attribute__((noinline)) static void vcopy(void *d,const void *s,unsigned n){
 for(unsigned i=0;i<n;i+=16) __asm__ volatile("ld.b $w0,0(%0)\n\tst.b $w0,0(%1)"::"r"((const char*)s+i),"r"((char*)d+i):"$w0","memory");
}
static int samples[1920] __attribute__((aligned(16)));
static unsigned long long energy_msa(void){unsigned long long sum=0;unsigned q[4] __attribute__((aligned(16)));for(unsigned i=0;i<1920;i+=4){__asm__ volatile("ld.w $w0,0(%0)\n\tmulv.w $w1,$w0,$w0\n\tst.w $w1,0(%1)"::"r"(samples+i),"r"(q):"$w0","$w1","memory");for(unsigned k=0;k<4;k++)sum+=q[k];}return sum;}
int main(void){
 for(unsigned i=0;i<1920;i++)samples[i]=(int)((i*37)%65536)-32768;unsigned long long ref=0;for(unsigned i=0;i<1920;i++)ref+=(unsigned)(samples[i]*samples[i]);if(energy_msa()!=ref)return 4;printf("msa_pcm_energy=pass samples=1920\n");
 unsigned x[4] __attribute__((aligned(16)))={1,2,3,4},y[4] __attribute__((aligned(16)))={0};
 __asm__ volatile("ld.w $w0,0(%0)\n\taddv.w $w1,$w0,$w0\n\tst.w $w1,0(%1)"::"r"(x),"r"(y):"$w0","$w1","memory");
 if(y[0]!=2||y[1]!=4||y[2]!=6||y[3]!=8)return 2;
 for(unsigned i=0;i<sizeof a;i++)a[i]=(i*37+11)&255;
 vcopy(b,a,sizeof a);if(memcmp(a,b,sizeof a))return 3;
 unsigned t=now();for(unsigned j=0;j<160;j++)memcpy(b,a,sizeof a);unsigned libc=now()-t;
 t=now();for(unsigned j=0;j<160;j++)vcopy(b,a,sizeof a);unsigned msa=now()-t;
 printf("msa_add=pass copy_match=pass bytes=%u repetitions=160 libc_us=%u msa_us=%u\n",(unsigned)sizeof a,libc,msa);
 return memcmp(a,b,sizeof a)!=0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");


