#include "spectrum_core.h"
#include "spectrum_tables.h"

static int scaled_product(int a,int b) { return (int)(((long long)a*b)/32768); }
static unsigned root(unsigned long long v) {
  unsigned long long bit=1ULL<<62,result=0;
  while(bit>v)bit>>=2;
  while(bit) { if(v>=result+bit){v-=result+bit;result=(result>>1)+bit;}else result>>=1;bit>>=2; }
  return (unsigned)result;
}
/* ALSA signed LE enums: S16_LE=2, S24_LE=6 (four-byte container),
 * S32_LE=10, S24_3LE=32. Explicitly reject other formats, including DSD. */
int disc_pcm_mono(const unsigned char *data,unsigned channels,unsigned format,short *out) {
  if(!data||!out||channels<1||channels>2)return 0;
  int bytes=format==2?2:format==32?3:(format==6||format==10)?4:0;
  if(!bytes)return 0;
  int sum=0;
  for(unsigned c=0;c<channels;c++) {
    const unsigned char *p=data+c*bytes;int sample;
    if(format==2)sample=(short)((unsigned)p[0]|(unsigned)p[1]<<8);
    else if(format==6||format==32)sample=(short)((unsigned)p[1]|(unsigned)p[2]<<8);
    else sample=(short)((unsigned)p[2]|(unsigned)p[3]<<8);
    sum+=sample;
  }
  *out=(short)(sum/(int)channels);return bytes*(int)channels;
}
void disc_spectrum_analyze(DiscSpectrumCore *s,const short *pcm,unsigned rate) {
  if(rate<8000||rate>768000){for(unsigned b=0;b<DISC_BANDS;b++)s->levels[b]=0;return;}
  for(unsigned i=0;i<DISC_FFT_SIZE;i++) {s->re[i]=scaled_product(pcm[i],disc_hann[i]);s->im[i]=0;}
  for(unsigned i=1,j=0;i<DISC_FFT_SIZE;i++) {
    unsigned bit=DISC_FFT_SIZE>>1;
    for(;j&bit;bit>>=1)j^=bit;
    j^=bit;
    if(i<j){int t=s->re[i];s->re[i]=s->re[j];s->re[j]=t;}
  }
  for(unsigned size=2;size<=DISC_FFT_SIZE;size<<=1) {
    unsigned step=DISC_FFT_SIZE/size;
    for(unsigned off=0;off<DISC_FFT_SIZE;off+=size)for(unsigned k=0;k<size/2;k++) {
      unsigned j=off+k,l=j+size/2,t=k*step;
      int tr=scaled_product(disc_cos[t],s->re[l])-scaled_product(disc_sin[t],s->im[l]);
      int ti=scaled_product(disc_cos[t],s->im[l])+scaled_product(disc_sin[t],s->re[l]);
      int a=s->re[j],b=s->im[j];
      s->re[j]=(a+tr)/2;s->im[j]=(b+ti)/2;s->re[l]=(a-tr)/2;s->im[l]=(b-ti)/2;
    }
  }
  for(unsigned k=0;k<DISC_FFT_SIZE/2;k++)s->magnitude[k]=root((long long)s->re[k]*s->re[k]+(long long)s->im[k]*s->im[k]);
  /* Logarithmic bands expressed as Q16 fractions of a 48kHz input. For other
   * rates convert the edges; avoid assuming host audio is always 192k/32bit. */
  for(unsigned b=0;b<DISC_BANDS;b++) {
    unsigned begin=disc_edges[b]*48000u/rate/64;
    unsigned end=disc_edges[b+1]*48000u/rate/64;
    if(begin<1)begin=1;
    if(end<=begin)end=begin+1;
    if(end>DISC_FFT_SIZE/2)end=DISC_FFT_SIZE/2;
    unsigned peak=0;for(unsigned k=begin;k<end;k++)if(s->magnitude[k]>peak)peak=s->magnitude[k];
    /* Approximate log2 amplitude with 8-bit fractions; FFT was scaled by N. */
    unsigned value=peak*4,exponent=0,tmp=value;
    while(tmp>1){tmp>>=1;exponent++;}
    int logq=value?(int)(exponent*256+((value-(1u<<exponent))*256/(1u<<exponent))):0;
    int level=(logq-1090)*255/(3840-1090);if(level<0)level=0;if(level>255)level=255;
    int delta=level-s->levels[b];
    if(delta>0)s->levels[b]+=(delta*3+3)/4;
    else if(delta<0)s->levels[b]-=(-delta+3)/4;
  }
}
