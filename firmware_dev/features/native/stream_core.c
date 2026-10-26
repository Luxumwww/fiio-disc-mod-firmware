#include "stream_core.h"
#include "fir_tables.h"
void disc_stream_reset(DiscStream *s){unsigned char *p=(unsigned char *)s;for(unsigned i=0;i<sizeof(*s);i++)p[i]=0;}
static void new_window(DiscStream *s){
 s->head=s->phase=s->fill=s->writepos=s->seen=0;
 for(unsigned c=0;c<2;c++)for(unsigned i=0;i<DISC_FIR_MAX;i++)s->history[c][i]=0;
 for(unsigned b=0;b<DISC_BANDS;b++){s->levels[b]=0;s->spectrum[0].levels[b]=s->spectrum[1].levels[b]=0;}
 s->db[0]=s->db[1]=-96;
}
int disc_stream_packet(DiscStream *s,const DiscPcmPacket *p,unsigned count){
 if(count<32||p->magic!=DISC_PCM_MAGIC||p->version!=1||p->channels<1||p->channels>2||!p->frames||p->frames>DISC_PCM_FRAMES||p->rate<8000||p->rate>768000)return 0;
 unsigned width=p->format==2?2:p->format==32?3:(p->format==6||p->format==10)?4:0;
 if(!width||count!=32+p->frames*p->channels*width)return 0;
 if(s->epoch!=p->epoch||s->next_position!=p->position||s->rate!=p->rate||s->format!=p->format||s->channels!=p->channels){
  new_window(s);s->dop=s->marker_run=s->bad_markers=s->last_marker=0;
  s->epoch=p->epoch;s->rate=p->rate;s->format=p->format;s->channels=p->channels;s->decimation=(p->rate+47999)/48000;
 }
 s->next_position=p->position+p->frames;s->accepted++;
 unsigned taps=s->decimation==1?1:16*s->decimation+1;
 for(unsigned f=0;f<p->frames;f++){
  const unsigned char *data=p->pcm+f*p->channels*width;
  /* DoP markers in the top byte of S24 or either common S32 packing. Detect
   * repeated alternating markers on both channels; encoded DSD is not FFT PCM. */
  unsigned marker=0;
  if(width>=3){
   for(unsigned k=2;k<width;k++){
    unsigned m=data[k];if(m!=5&&m!=250)continue;
    if(p->channels==2&&data[width+k]!=m)continue;
    marker=m;break;
   }
  }
  if(marker&&marker!=s->last_marker){s->marker_run++;s->bad_markers=0;}
  else{s->marker_run=marker?1:0;s->bad_markers++;}
  s->last_marker=marker;
  if(s->marker_run>=8&&!s->dop){s->dop=1;new_window(s);}
  if(s->dop&&s->bad_markers>=8){s->dop=0;new_window(s);}
  if(s->dop)continue;
  for(unsigned c=0;c<p->channels;c++){
   short sample=0;disc_pcm_mono(data+c*width,1,p->format,&sample);s->history[c][s->head]=sample;
  }
  unsigned newest=s->head;s->head=(s->head+1)%taps;
  if(++s->phase<s->decimation)continue;s->phase=0;
  for(unsigned c=0;c<p->channels;c++){
   int sample;
   if(taps==1)sample=s->history[c][newest];
   else{
    long long sum=0;unsigned j=newest;
    for(unsigned k=0;k<taps;k++){sum+=(int)s->history[c][j]*(int)disc_fir[s->decimation][k];j=j?j-1:taps-1;}
    sample=(int)(sum/32768);
   }
   if(sample>32767)sample=32767;if(sample<-32768)sample=-32768;
   s->window[c][s->writepos]=(short)sample;
  }
  s->writepos=(s->writepos+1)%DISC_FFT_SIZE;if(s->fill<DISC_FFT_SIZE)s->fill++;s->seen++;
 }
 return 1;
}
int disc_stream_analyze(DiscStream *s){
 if(s->dop||s->fill<DISC_FFT_SIZE||s->seen<256)return 0;
 short ordered[DISC_FFT_SIZE];
 for(unsigned c=0;c<s->channels;c++){
  unsigned long long power=0;
  for(unsigned i=0;i<DISC_FFT_SIZE;i++){ordered[i]=s->window[c][(s->writepos+i)%DISC_FFT_SIZE];power+=(long long)ordered[i]*ordered[i];}
  unsigned value=0,bit=1u<<30;power/=DISC_FFT_SIZE;
  while(bit>power)bit>>=2;
  while(bit){if(power>=value+bit){power-=value+bit;value=(value>>1)+bit;}else value>>=1;bit>>=2;}
  unsigned exponent=0,tmp=value;while(tmp>1){tmp>>=1;exponent++;}
  int logq=value?(int)(exponent*256+(value-(1u<<exponent))*256/(1u<<exponent)):0;
  int db=value?(logq-15*256)*602/25600:-96;if(db<-96)db=-96;if(db>0)db=0;s->db[c]=db;
  disc_spectrum_analyze(&s->spectrum[c],ordered,s->rate/s->decimation);
 }
 if(s->channels==1)s->db[1]=s->db[0];
 for(unsigned b=0;b<DISC_BANDS;b++){
  unsigned char l=s->spectrum[0].levels[b],r=s->channels==2?s->spectrum[1].levels[b]:0;
  s->levels[b]=l>r?l:r; /* preserves either channel, including opposite phase */
 }
 s->seen=0;return 1;
}
