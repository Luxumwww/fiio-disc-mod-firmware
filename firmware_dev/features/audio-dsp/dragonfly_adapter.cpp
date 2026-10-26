/* Dragonfly Hall processing composition (GPL-3.0-or-later), Freeverb3 core
 * (GPL-2.0-or-later). Original DSP.cpp and full source/licenses are retained.
 * No VST/GUI, C++ standard runtime, exceptions, or new audio thread. */
#include "dragonfly_adapter.h"
#include "third_party/dragonfly-port/disc_cpp_compat.hpp"
extern "C" int _setjmp(void *);
extern "C" [[noreturn]] void longjmp(void *,int);
#include "freeverb/earlyref.hpp"
#include "freeverb/zrev2.hpp"
extern "C" int pthread_mutex_lock(void *);
extern "C" int pthread_mutex_unlock(void *);
static unsigned alloc_mutex[10];
union Header {struct {Header *prev,*next;size_t bytes;} links;long double alignment;};
static Header *allocations;
static unsigned long long allocation_exit[128];
static bool constructing;static unsigned allocation_bytes;
#ifdef DISC_FV_ALLOC_TEST
static unsigned fail_at,alloc_attempt,outstanding;
extern "C" void disc_fv_test_fail(unsigned at){fail_at=at;alloc_attempt=0;}
extern "C" unsigned disc_fv_test_outstanding(){return outstanding;}
#endif
[[noreturn]] void disc_fv_oom(){longjmp(allocation_exit,1);}
void *disc_fv_alloc(size_t bytes){
#ifdef DISC_FV_ALLOC_TEST
 if(constructing&&++alloc_attempt==fail_at)disc_fv_oom();
#endif
 Header *h=(Header *)malloc(sizeof(Header)+bytes);
 if(!h)disc_fv_oom();
#ifdef DISC_FV_ALLOC_TEST
 outstanding++;
#endif
 h->links.prev=0;h->links.next=0;h->links.bytes=bytes+sizeof(Header);
 if(constructing){allocation_bytes+=h->links.bytes;h->links.next=allocations;if(allocations)allocations->links.prev=h;allocations=h;}
 return h+1;
}
void disc_fv_free(void *p){if(!p)return;Header *h=(Header *)p-1;
 if(constructing){allocation_bytes-=h->links.bytes;if(h->links.prev)h->links.prev->links.next=h->links.next;else allocations=h->links.next;if(h->links.next)h->links.next->links.prev=h->links.prev;}
#ifdef DISC_FV_ALLOC_TEST
 outstanding--;
#endif
 free(h);
}
void *operator new(__SIZE_TYPE__ n){return disc_fv_alloc(n);}
void *operator new[](__SIZE_TYPE__ n){return disc_fv_alloc(n);}
void operator delete(void *p) noexcept {disc_fv_free(p);}
void operator delete[](void *p) noexcept {disc_fv_free(p);}
extern "C" void __cxa_pure_virtual(){abort();}
struct Dragonfly {
 fv3::earlyref_f early;fv3::zrev2_f late;
 unsigned memory_bytes;unsigned factor,phase,input_pos,wet_pos,taps;
 float input[2][129],wet[2][17],down[129],up[8][17];
 float early_level,late_level,dry_level;
};
extern "C" void *disc_dragonfly_create_configured(unsigned rate,const float *p){
 pthread_mutex_lock(alloc_mutex);constructing=true;allocations=0;allocation_bytes=0;
 if(_setjmp(allocation_exit)){
  while(allocations){Header *next=allocations->links.next;free(allocations);allocations=next;
#ifdef DISC_FV_ALLOC_TEST
   outstanding--;
#endif
  }
  constructing=false;pthread_mutex_unlock(alloc_mutex);return 0;
 }
 Dragonfly *d=new Dragonfly;
 d->early.setMuteOnChange(false);d->late.setMuteOnChange(false);
 d->early.loadPresetReflection(FV3_EARLYREF_PRESET_1);
 d->early.setdryr(0);d->early.setwet(0);d->early.setLRDelay(.3);
 d->early.setLRCrossApFreq(750,4);d->early.setDiffusionApFreq(150,4);
 // Only the band-limited wet branch runs at <=48k; full-rate dry is retained.
 // Symmetric windowed-sinc downsampling and polyphase reconstruction avoid
 // aliasing from simply dropping samples. State persists across audio blocks.
 d->factor=(rate+47999)/48000;if(d->factor<1)d->factor=1;
 float core_rate=(float)rate/d->factor;
 d->early.setSampleRate(core_rate);d->late.setSampleRate(core_rate);
 d->phase=d->input_pos=d->wet_pos=0;d->taps=16*d->factor+1;
 memset(d->input,0,sizeof(d->input));memset(d->wet,0,sizeof(d->wet));
 if(d->factor>1){
  float sum=0,cut=.42f/d->factor;int center=8*d->factor;
  for(unsigned i=0;i<d->taps;i++){
   float x=(int)i-center;
   float sinc=x==0?2*cut:sinf(2*M_PI*cut*x)/(M_PI*x);
   float w=.42f-.5f*cosf(2*M_PI*i/(d->taps-1))+.08f*cosf(4*M_PI*i/(d->taps-1));
   d->down[i]=sinc*w;sum+=d->down[i];
  }
  for(unsigned i=0;i<d->taps;i++)d->down[i]/=sum;
  for(unsigned phase=0;phase<d->factor;phase++){
   sum=0;for(unsigned k=0;k<17;k++){
    unsigned i=k*d->factor+phase;float x=(int)i-center;
    float sinc=x==0?2*cut:sinf(2*M_PI*cut*x)/(M_PI*x);
    float w=i>=d->taps?0:.42f-.5f*cosf(2*M_PI*i/(d->taps-1))+.08f*cosf(4*M_PI*i/(d->taps-1));
    d->up[phase][k]=sinc*w;sum+=d->up[phase][k];
   }
   for(unsigned k=0;k<17;k++)d->up[phase][k]/=sum;
  }
 }
 // Moderately large, wide hall: retain attack through early reflection and dry.
 d->early.setRSFactor(p?p[0]:3.2);d->late.setRSFactor(.4);
 d->early.setwidth(p?p[4]:1);d->late.setwidth(p?p[4]:1);
 d->late.setdryr(0);d->late.setwet(0);d->late.setPreDelay(p?p[2]:12);
 d->late.setidiffusion1(.64);d->late.setapfeedback(.64);
 d->early.setoutputhpf(20);d->late.setoutputhpf(20);
 float cutoff=p?p[3]:10000;if(cutoff>core_rate*.4f)cutoff=core_rate*.4f;
 if(!p)cutoff=core_rate<24000?core_rate*.4:10000;
 d->early.setoutputlpf(cutoff);d->late.setoutputlpf(cutoff);
 d->late.setxover_low(300);d->late.setrt60_factor_low(1.0);
 d->late.setxover_high(rate<12000?rate*.35:4000);d->late.setrt60_factor_high(.65);
 d->late.setspin(1.2);d->late.setwander(10);d->late.setspinfactor(.18);d->late.setlfofactor(.18);d->late.setrt60(p?p[1]:1.6);
 d->early_level=p?p[5]:.30f;d->late_level=p?p[6]:.85f;d->dry_level=p?p[7]:.25f;
 d->early.mute();d->late.mute();
 allocations=0;d->memory_bytes=allocation_bytes;constructing=false;pthread_mutex_unlock(alloc_mutex);return d;
}
extern "C" void *disc_dragonfly_create(unsigned rate){return disc_dragonfly_create_configured(rate,0);}
extern "C" void disc_dragonfly_destroy(void *p){delete (Dragonfly *)p;}
extern "C" void disc_dragonfly_reset(void *p){Dragonfly *d=(Dragonfly *)p;d->early.mute();d->late.mute();d->phase=d->input_pos=d->wet_pos=0;memset(d->input,0,sizeof(d->input));memset(d->wet,0,sizeof(d->wet));}
extern "C" void disc_dragonfly_sample(void *p,float *io){
 Dragonfly *d=(Dragonfly *)p;float e[2],l[2],in[2],filtered[2]={io[0],io[1]};
 if(d->factor>1){
  for(unsigned c=0;c<2;c++)d->input[c][d->input_pos]=io[c];
  if(d->phase==0){filtered[0]=filtered[1]=0;
   unsigned index=d->input_pos;
   for(unsigned k=0;k<d->taps;k++){for(unsigned c=0;c<2;c++)filtered[c]+=d->input[c][index]*d->down[k];if(index==0)index=d->taps;index--;}
  }
  if(++d->input_pos==d->taps)d->input_pos=0;
 }
 if(d->phase==0){
  d->early.processreplace(filtered,filtered+1,e,e+1,1);
  in[0]=filtered[0]+.2f*e[0];in[1]=filtered[1]+.2f*e[1];
  d->late.processreplace(in,in+1,l,l+1,1);
  for(unsigned c=0;c<2;c++)d->wet[c][d->wet_pos]=d->early_level*e[c]+d->late_level*l[c];
 }
 for(unsigned c=0;c<2;c++){
  float w=d->wet[c][d->wet_pos];
  if(d->factor>1){w=0;unsigned index=d->wet_pos;for(unsigned k=0;k<17;k++){w+=d->wet[c][index]*d->up[d->phase][k];if(index==0)index=17;index--;}}
  io[c]=d->dry_level*io[c]+w;
 }
 if(++d->phase==d->factor){d->phase=0;if(++d->wet_pos==17)d->wet_pos=0;}
}

extern "C" unsigned disc_dragonfly_memory(void *p){return p?((Dragonfly *)p)->memory_bytes:0;}
